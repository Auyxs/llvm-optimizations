#include "LoopFusion.h"
using namespace llvm;

PreservedAnalyses LoopFusionOpt::run(Function &F, FunctionAnalysisManager &FAM) {
  outs() << "\nRunning LoopFusionOpt on function: " << F.getName() << "\n";

  auto &LI = FAM.getResult<LoopAnalysis>(F);
  bool changed = false;

  const std::vector<Loop*>& topLevelLoops = LI.getTopLevelLoops();
  changed = runOnLoops(F, FAM, topLevelLoops);

  return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool LoopFusionOpt::runOnLoops(Function &F, FunctionAnalysisManager &FAM, const std::vector<Loop*> &Loops) {
  bool changed = false;

  Loop *prev = nullptr;
  for (auto rit = Loops.rbegin(); rit != Loops.rend(); ++rit) {
    Loop* curr = *rit; 
    
    if (prev && isOptimizable(F, FAM, prev, curr)) {
        outs() << "\noptimizable loops: " << prev << " " << curr << "\n";
        prev = optimizeLoop(F, FAM, prev, curr);
        changed = true;
    } else {
        prev = curr;
    }
  }
  //recurse on subloops
  for (Loop *L : Loops) {
    const std::vector<Loop*> &SubLoops = L->getSubLoops();

    if (SubLoops.size() >= 2)
      changed |= runOnLoops(F, FAM, L->getSubLoops());
  }

  return changed;
}

Loop* LoopFusionOpt::optimizeLoop(Function &F, FunctionAnalysisManager &FAM, Loop *prev, Loop *curr) {
  auto &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  auto &LI = FAM.getResult<LoopAnalysis>(F);

  auto *prevLatch = prev->getLoopLatch();
  auto *prevBodyEntry = prevLatch->getSinglePredecessor();
  auto *prevHeader = prev->getHeader();
  auto *prevPreheader = prev->getLoopPreheader();
  auto *prevExit = prev->getExitBlock();
  auto *prevGuard = prev->getLoopGuardBranch();

  auto *currLatch = curr->getLoopLatch();
  auto *currBodyEntry = currLatch->getSinglePredecessor();
  auto *currHeader = curr->getHeader();
  auto *currPreheader = curr->getLoopPreheader();
  auto *currExit = curr->getExitBlock();

  // Replace Induction Variable
  auto *prevIV = prev->getCanonicalInductionVariable();
  auto *currIV = curr->getCanonicalInductionVariable();

  currIV->replaceAllUsesWith(prevIV);
  currIV->eraseFromParent();

  // Move PHIs from currHeader to prevHeader
  SmallVector<PHINode *, 8> currHeaderPHIs;
  for (auto &I : *currHeader) {
    if (auto *PHI = dyn_cast<PHINode>(&I)) {
      currHeaderPHIs.push_back(PHI);
    }
  }

  auto *insertionPoint = prevHeader->getFirstNonPHI();
  for (auto *PHI : currHeaderPHIs) {

    // handle when PHI depends on a PHI from the previous loop
    Value *in0 = PHI->getIncomingValue(0);
    Value *in1 = PHI->getIncomingValue(1);

    if (auto *prevPHI = dyn_cast<PHINode>(in0)) {
      if (prevHeader == prevPHI->getParent()) {
        auto prevInstruction = dyn_cast<Instruction>(prevPHI->getIncomingValue(1));
        auto userInstruction = dyn_cast<Instruction>(in1);
        userInstruction->setOperand(0, prevInstruction);
        prevPHI->setIncomingValue(1, in1);
        PHI->replaceAllUsesWith(prevPHI);
        PHI->eraseFromParent();
        
        continue;
      }
    }

    PHI->moveBefore(insertionPoint);
    for (unsigned i = 0, e = PHI->getNumIncomingValues(); i != e; ++i) {
      if (PHI->getIncomingBlock(i) == currPreheader) {
        PHI->setIncomingBlock(i, prevPreheader);
      } else if (PHI->getIncomingBlock(i) == currLatch) {
        PHI->setIncomingBlock(i, prevLatch);
      }
    }
  }

  // Redirect control flow
  prevHeader->getTerminator()->replaceSuccessorWith(prevExit, currExit);
  prevBodyEntry->getTerminator()->replaceSuccessorWith(prevLatch, currBodyEntry);
  currBodyEntry->getTerminator()->replaceSuccessorWith(currLatch, prevLatch);
  currHeader->getTerminator()->replaceSuccessorWith(currBodyEntry, currLatch);
  if (prevGuard) {
    prevGuard->setSuccessor(1, currExit);
  }

  // move curr body blocks before prev latch
  for (auto *BB : curr->blocks()) {
    if (BB == currHeader || BB == currLatch || BB == currExit)
      continue;

    BB->moveBefore(prevLatch);
  }

  // Update loop structure
  prev->addBasicBlockToLoop(currBodyEntry, LI);
  curr->removeBlockFromLoop(currBodyEntry);
  LI.erase(curr);
  removeUnreachableBlocks(F);

  return prev;
}


SetVector<Instruction*> getBodyInstructions(Loop *L){
  SetVector<Instruction*> instructions;
  for (auto *BB : L->blocks()) {
    for (Instruction &I : *BB) {
      instructions.insert(&I);
    }
  }
  return instructions;
}

bool LoopFusionOpt::isOptimizable(Function &F, FunctionAnalysisManager &FAM, Loop *prev, Loop *curr){
  auto isAdjacent = [&]() -> bool {
    auto prevExitBB = prev->isGuarded() ? 
        prev->getExitBlock()->getSingleSuccessor()  
        : prev->getExitBlock();

    if (not prevExitBB) return false; 

    auto nextEntryBB = curr->isGuarded() ?
        curr->getLoopGuardBranch()->getParent()
        : curr->getLoopPreheader();;

    return prevExitBB == nextEntryBB;
};

  auto hasSameLoopTripCount = [&]() -> bool {
    ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
    const SCEV *TripCountPrev = SE.getBackedgeTakenCount(prev);
    const SCEV *TripCountCurr = SE.getBackedgeTakenCount(curr);

    if (isa<SCEVCouldNotCompute>(TripCountPrev) || 
        isa<SCEVCouldNotCompute>(TripCountCurr))
        return false;
    
    return TripCountCurr == TripCountPrev;
  };

  auto isControlFlowEq = [&]() -> bool {
    DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = FAM.getResult<PostDominatorTreeAnalysis>(F);

    BasicBlock *H0 = prev->getLoopPreheader();
    BasicBlock *H1 = curr->getLoopPreheader();

    // If both loops are guarded, check if their guard conditions are the same
    bool condEq = true;
    if (prev->isGuarded() && curr->isGuarded()) {
      BranchInst *guardBranch0 = prev->getLoopGuardBranch();
      BranchInst *guardBranch1 = curr->getLoopGuardBranch();

      H0 = guardBranch0->getParent();
      H1 = guardBranch1->getParent();

      if (guardBranch0->isConditional() && guardBranch1->isConditional()) {
        if (auto *icmp0 = dyn_cast<ICmpInst>(guardBranch0->getCondition())) {
          if (auto *icmp1 = dyn_cast<ICmpInst>(guardBranch1->getCondition())) {
            condEq = icmp0->isIdenticalTo(icmp1);
          }
        }
      }
    }

    return DT.dominates(H0, H1) && PDT.dominates(H1, H0) && condEq;
  };

  auto hasNotDependencies = [&]() -> bool {
    // auto &DI = FAM.getResult<DependenceAnalysis>(F);

    // for (auto *BB : prev->getBlocks()) {
    //   for (auto &I : *BB) {
    //     for (auto *BB2 : curr->getBlocks()) {
    //       for (auto &I2 : *BB2) {
    //         if (DI.depends(&I, &I2, true)) {
    //           errs() << "Dependency " << I << I2 << "\n";
    //           return false;
    //         }
    //       }
    //     }
    //   }
    // }
    return true;
  };

  return isAdjacent() && hasSameLoopTripCount() && isControlFlowEq() && hasNotDependencies();
}


PassPluginLibraryInfo getLocalOptPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "LoopFusionOpt",
    "v1.0",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "LoopFusion-opt") {
            FPM.addPass(LoopFusionOpt());
            return true;
          }
          return false;
        });
    }
  };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getLocalOptPluginInfo();
}
