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
        prev = fuseLoops(F, FAM, prev, curr);
        changed = true;
    } else {
        prev = curr;
    }
  }

  return changed;
}

Loop* LoopFusionOpt::fuseLoops(Function &F, FunctionAnalysisManager &FAM, Loop *prev, Loop *curr) {
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  auto &LI = FAM.getResult<LoopAnalysis>(F);

  auto *prevPreheader = prev->getLoopPreheader();
  auto *prevLatch = prev->getLoopLatch();
  auto *prevBody = prevLatch->getSinglePredecessor();
  auto *prevHeader = prevPreheader->getSingleSuccessor();
  auto *prevGuard = prev->getLoopGuardBranch();
  auto *prevExit = prev->getExitBlock();

  auto *currPreheader = curr->getLoopPreheader();
  auto *currLatch = curr->getLoopLatch();
  auto *currBody = currLatch->getSinglePredecessor();
  auto *currHeader = currPreheader->getSingleSuccessor();
  auto *currExit = curr->getExitBlock();
  auto *currGuard = curr->getLoopGuardBranch();

  // Replace Induction Variable
  auto *prevIV = prev->getInductionVariable(SE);
  auto *currIV = curr->getInductionVariable(SE);
  currIV->replaceAllUsesWith(prevIV);
  currIV->eraseFromParent();

  // Adjust PHInodes
  auto isLCSSAPhi = [](llvm::PHINode *PHI, llvm::Loop *L) -> bool {
    if (L->contains(PHI->getParent())) return false;

    for (unsigned i = 0, e = PHI->getNumIncomingValues(); i < e; ++i) {
      BasicBlock *Pred = PHI->getIncomingBlock(i);
      if (!L->contains(Pred))
        return false;
    }

    return true; // It's an LCSSA PHI for loop L
  };
  currHeader->replacePhiUsesWith(currLatch, prevLatch);
  currHeader->replacePhiUsesWith(currPreheader, prevPreheader);
  currPreheader->replacePhiUsesWith(currPreheader->getSinglePredecessor(), prevBody);
  currExit->replacePhiUsesWith(currLatch, prevLatch);

  SmallVector<PHINode *, 8> currHeaderPHIs;
  for (auto &I : *currHeader) {
    if (auto *PHI = dyn_cast<PHINode>(&I)) {
      currHeaderPHIs.push_back(PHI);
    }
  }
  SmallVector<PHINode *, 8> prevHeaderPHIs;
  for (auto &I : *prevHeader) {
    if (auto *PHI = dyn_cast<PHINode>(&I)) {
      prevHeaderPHIs.push_back(PHI);
    }
  }
  auto *insertionPoint = prevHeader->getFirstNonPHI();
  for (auto *PHI : currHeaderPHIs) {
    // handle when PHI depends on a PHI from the previous loop
    Value *in0 = PHI->getIncomingValue(0);
    Value *in1 = PHI->getIncomingValue(1);

    if (auto *lcssaPHI = dyn_cast<PHINode>(in0)) {
      if (prevExit == lcssaPHI->getParent() && isLCSSAPhi(lcssaPHI, prev)) {
        auto lcssaValue = lcssaPHI->getIncomingValue(0);

        for (PHINode *prevPHI : prevHeaderPHIs){
          if (prevPHI->getIncomingValue(1) == lcssaValue)
            prevPHI->setIncomingValue(1, in1);
        }

        PHI->replaceAllUsesWith(lcssaValue);
        PHI->eraseFromParent();
        lcssaPHI->eraseFromParent();
        
        continue;
      }
    }

    PHI->moveBefore(insertionPoint);
  }

  // move lcssa instruction form prevexit to currexit
  Instruction *movePoint = currExit->getFirstNonPHI();
  SmallVector<PHINode *> lcssaToMove;
  for (Instruction &I : *prevExit) {
    errs() << "Moving phi: " << I << "\n";
    if (auto *phi = dyn_cast<PHINode>(&I)) {
      lcssaToMove.push_back(phi);
      phi->setIncomingBlock(0, prevLatch);
      errs() << "Moving phi: " << *phi << "\n";
    }
  }

  for (PHINode *phi : lcssaToMove) phi->moveBefore(movePoint);

  if (prevGuard && currGuard) {
    prevGuard->setSuccessor(1, currExit->getSingleSuccessor());

    // // Move instructions from currGuard to currExit's successor
    // BasicBlock *guardDest = currExit->getSingleSuccessor();
    // Instruction *insertionPoint = guardDest->getFirstNonPHI();

    // currGuard->getParent()->replacePhiUsesWith(prevExit, currExit);
    // SmallVector<Instruction *> toMove;
    // for (Instruction &I : *currGuard->getParent()) {
    //   if (!I.isTerminator() )
    //     toMove.push_back(&I);
    // }

    // for (Instruction *inst : toMove) inst->moveBefore(insertionPoint);
  }

  // Redirect control flow
  prevLatch->getTerminator()->setSuccessor(1, currExit);
  prevBody->getTerminator()->replaceSuccessorWith(prevLatch, currPreheader);
  currBody->getTerminator()->replaceSuccessorWith(currLatch, prevLatch);
  currLatch->getTerminator()->replaceSuccessorWith(currExit, currLatch); // loop to itself (BB to be removed)

  // clean up and move curr blocks
  removeUnreachableBlocks(F);
  LI.erase(curr);

  SmallVector<BasicBlock*> currBlocks;
  for (BasicBlock *bb : curr->getBlocks()) {
    if (bb != currLatch)
      currBlocks.push_back(bb);
  }
  currBlocks.push_back(currPreheader);
  currBlocks.push_back(currHeader);

  for (auto *bb : currBlocks){
    prev->addBasicBlockToLoop(bb, LI);
    bb->moveBefore(prevLatch);
  }

  return prev;
}

bool LoopFusionOpt::isOptimizable(Function &F, FunctionAnalysisManager &FAM, Loop *prev, Loop *curr){
  auto isAdjacent = [&]() -> bool {
    auto prevExitBB = prev->isGuarded() ? 
        prev->getExitBlock()->getSingleSuccessor()  
        : prev->getExitBlock();

    if (not prevExitBB) return false; 

    auto nextEntryBB = curr->isGuarded() ?
        curr->getLoopGuardBranch()->getParent()
        : curr->getLoopPreheader();

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

    BasicBlock *H0 = prev->getHeader();
    BasicBlock *H1 = curr->getHeader();

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
