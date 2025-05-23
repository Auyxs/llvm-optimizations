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
    return true;
  };

  auto hasSameLoopTripCount = [&]() -> bool {
    return true;
  };

  auto isControlFlowEq = [&]() -> bool {
    return true;
  };

  auto hasNotDependencies = [&]() -> bool {
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
