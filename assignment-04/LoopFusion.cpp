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
void deleteBlock(BasicBlock* block){
  while (!block->empty()) {
    Instruction &inst = block->back();
    inst.eraseFromParent();
  }

  block->eraseFromParent();
}

Loop* LoopFusionOpt::fuseLoops(Function &F, FunctionAnalysisManager &FAM, Loop *prev, Loop *curr) {
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  auto &LI = FAM.getResult<LoopAnalysis>(F);

  // === Fetch loop components ===
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

  // === Replace induction variable ===
  auto *prevIV = prev->getInductionVariable(SE);
  auto *currIV = curr->getInductionVariable(SE);
  currIV->replaceAllUsesWith(prevIV);
  currIV->eraseFromParent();

  // === Adjust PHI nodes ===
  auto isLCSSAPhi = [](PHINode *PHI, Loop *L) {
    if (L->contains(PHI->getParent())) return false;
    for (unsigned i = 0, e = PHI->getNumIncomingValues(); i < e; ++i)
      if (!L->contains(PHI->getIncomingBlock(i)))
        return false;
    return true;
  };

  currHeader->replacePhiUsesWith(currLatch, prevLatch);
  currHeader->replacePhiUsesWith(currPreheader, prevPreheader);
  currPreheader->replacePhiUsesWith(currPreheader->getSinglePredecessor(), prevBody);
  currExit->replacePhiUsesWith(currLatch, prevLatch);

  SmallVector<PHINode *, 8> currHeaderPHIs;
  for (auto &I : *currHeader)
    if (auto *PHI = dyn_cast<PHINode>(&I))
      currHeaderPHIs.push_back(PHI);

  SmallVector<PHINode *, 8> prevHeaderPHIs;
  for (auto &I : *prevHeader)
    if (auto *PHI = dyn_cast<PHINode>(&I))
      prevHeaderPHIs.push_back(PHI);

  Instruction *insertBefore = prevHeader->getFirstNonPHI();
  for (auto *PHI : currHeaderPHIs) {
    Value *in0 = PHI->getIncomingValue(0);
    Value *in1 = PHI->getIncomingValue(1);

    if (auto *lcssaPHI = dyn_cast<PHINode>(in0)) {
      if (prevExit == lcssaPHI->getParent() && isLCSSAPhi(lcssaPHI, prev)) {
        Value *lcssaValue = lcssaPHI->getIncomingValue(0);

        for (auto *prevPHI : prevHeaderPHIs)
          if (prevPHI->getIncomingValue(1) == lcssaValue)
            prevPHI->setIncomingValue(1, in1);

        PHI->replaceAllUsesWith(lcssaValue);
        PHI->eraseFromParent();
        lcssaPHI->eraseFromParent();
        continue;
      }
    }

    PHI->moveBefore(insertBefore);
  }

  // === Move LCSSA phis from prevExit to currExit ===
  Instruction *movePoint = currExit->getFirstNonPHI();
  SmallVector<PHINode *> lcssaToMove;
  for (Instruction &I : *prevExit)
    if (auto *phi = dyn_cast<PHINode>(&I)) {
      phi->setIncomingBlock(0, prevLatch);
      lcssaToMove.push_back(phi);
    }

  for (auto *phi : lcssaToMove)
    phi->moveBefore(movePoint);

  // === Handle guards and exits ===
  if (prevGuard && currGuard) {
    BasicBlock *guardDest = currExit->getSingleSuccessor();

    prevGuard->setSuccessor(1, guardDest);
    guardDest->replacePhiUsesWith(currGuard->getParent(), prevGuard->getParent());

    currGuard->replaceSuccessorWith(guardDest, currGuard->getParent());
    prevExit->getTerminator()->setSuccessor(0, prevExit); 

    Instruction *insertPt = guardDest->getFirstNonPHI();
    SmallVector<Instruction *> toMove;

    for (Instruction &I : *currGuard->getParent())
      if (!I.isTerminator() && &I != currGuard->getCondition())
        toMove.push_back(&I);

    for (Instruction *inst : toMove) inst->moveBefore(insertPt);
    guardDest->replacePhiUsesWith(prevExit, currExit);

    deleteBlock(currGuard->getParent());
    deleteBlock(prevExit);
  }

  // === Redirect control flow ===
  prevLatch->getTerminator()->setSuccessor(1, currExit);
  prevBody->getTerminator()->replaceSuccessorWith(prevLatch, currPreheader);
  currBody->getTerminator()->replaceSuccessorWith(currLatch, prevLatch);
  currLatch->getTerminator()->replaceSuccessorWith(currExit, currLatch);
  deleteBlock(currLatch);

  // === Merge curr blocks into prev loop ===
  SmallVector<BasicBlock *> currBlocks;
  currBlocks.push_back(currPreheader);
  currBlocks.push_back(currHeader);

  for (auto *bb : currBlocks) {
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
  
  auto getInstructionPointer = [&](Instruction *Inst) -> Value * {
    if (auto *storeInst = dyn_cast<StoreInst>(Inst)) {
      return storeInst->getPointerOperand();
    }

    if (auto *loadInst = dyn_cast<LoadInst>(Inst)) {
      return loadInst->getPointerOperand();
    }

    return nullptr;
  };

  auto hasNotDependencies = [&]() -> bool {
    auto &DI = FAM.getResult<DependenceAnalysis>(F);
    AliasAnalysis &AA = FAM.getResult<AAManager>(F);
    ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
    const DataLayout &DL = F.getParent()->getDataLayout();
      for (auto *BB : prev->getBlocks()) {
        for (auto &I : *BB) {
          for (auto *BB2 : curr->getBlocks()) {
            for (auto &I2 : *BB2) {
              if (auto Dep = DI.depends(&I, &I2, true)) {

                outs() << "Istruzioniii: " << I << " " << I2 << "\n";
                Value *Pointer1 = getInstructionPointer(&I);
                Value *Pointer2 = getInstructionPointer(&I2);
                const SCEV *Expr1 = SE.getSCEV(Pointer1);
                const SCEV *Expr2 = SE.getSCEV(Pointer2);
                const SCEVAddRecExpr *AR1 = dyn_cast<SCEVAddRecExpr>(Expr1);
                const SCEVAddRecExpr *AR2 = dyn_cast<SCEVAddRecExpr>(Expr2);

                if(AR1 && AR2 && (AR1->getStepRecurrence(SE) == AR2->getStepRecurrence(SE))){
                  const SCEV *Dist = SE.getMinusSCEV(AR1->getStart(), AR2->getStart());

                  if(auto *ConstDist = dyn_cast<SCEVConstant>(Dist)){
                    const APInt &ByteOffset = ConstDist->getAPInt();

                    Type *ElemTy = isa<GetElementPtrInst>(Pointer1) ?
                    cast<GetElementPtrInst>(Pointer1)->getResultElementType()
                    : cast<GetElementPtrInst>(Pointer2)->getResultElementType();

                    uint64_t ElemSize = DL.getTypeAllocSize(ElemTy);
                    int64_t ElementOffset = ByteOffset.getSExtValue() / (int64_t)ElemSize;
                    outs() << "Distanza: " << ElementOffset << "\n";
                    if(ElementOffset < 0) return false;
                  }
                }
              }
            }
          }
        }
      }
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
