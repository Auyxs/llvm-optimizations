#include "LICMopt.h"
using namespace llvm;

PreservedAnalyses LICMopt::run(Function &F, FunctionAnalysisManager &FAM) {
  outs() << "\n" << "Running LICMopt on function: " << F.getName() << "\n";

  auto &LI = FAM.getResult<LoopAnalysis>(F);
  auto &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  bool changed = false;

  for (Loop *L : LI) 
    changed |= runOnLoop(L, DT);

  return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool LICMopt::runOnLoop(Loop *L, DominatorTree &DT) {
  BasicBlock *preheader = L->getLoopPreheader();
  if (!preheader) return false;
  SetVector<Instruction*> movable, moved;

  SmallVector<BasicBlock*> ExitBlocks;
  L->getExitBlocks(ExitBlocks);

  // Check if an instruction is loop-invariant
  auto isLoopInvariant = [&](Instruction &I) -> bool {
    if (!I.isBinaryOp())
      return false;

    for (Value *op : I.operands()) {
      if (isa<Constant>(op) || isa<Argument>(op)) continue;

      if (auto *OpInst = dyn_cast<Instruction>(op)) {
        if (isa<PHINode>(OpInst))
          return false;

        if (!L->contains(OpInst) || movable.contains(OpInst))
          continue;

        return false;
      }
    }
    return true;
  };

  // Collect loop invariants and movable instructions
  for (BasicBlock *BB : depth_first(L->getHeader())) {
    if (!L->contains(BB)) continue;

    for (Instruction &I : *BB) {
      if (isLoopInvariant(I) && isSafeToMove(I, L, DT, ExitBlocks)) {
        movable.insert(&I);
        outs() << "Found movable loopInvariant: " << I << "\n";
      }
    }
  }

  // Check for not moved dependencies 
  auto hasUnmovedDependencies = [&](Instruction *I) -> bool {
    for (Value *op : I->operands()) {
      if (auto *opInst = dyn_cast<Instruction>(op)) {
        if (L->contains(opInst) && !moved.contains(opInst))
          return true; // Dependency still inside loop
      }
    }
    return false;
  };

  // Move instructions
  for (Instruction *I : movable) {
    if (!hasUnmovedDependencies(I)) {
      I->moveBefore(preheader->getTerminator());
      moved.insert(I);
      outs() << "Moved to preheader: " << *I << "\n";
    }
  }

  return !moved.empty();
}

bool LICMopt::isSafeToMove(Instruction &I, Loop *L, DominatorTree &DT, SmallVector<BasicBlock*> ExitBlocks){
  // Instruction is not used outside loop
  auto isDeadOutsideLoop = [&](Instruction &I) -> bool {
    for (User *user : I.users()) {
      if (Instruction *userInstr = dyn_cast<Instruction>(user)) {
        if (!L->contains(userInstr)) {
          outs() << "NOT dead after: " << I << "\n";
          return false;
        }
      }
    }
    outs() << "dead after: " << I << "\n";
    return true;
  };

  // Check if instruction will execute before any possible loop exit.
  auto dominatesAllExits = [&](Instruction &I) -> bool {
    if (ExitBlocks.empty()) {
      outs() << "No exit blocks for: " << I << "\n";
      return false;
    }

    for (BasicBlock *Exit : ExitBlocks) {
      if (!DT.dominates(I.getParent(), Exit)) {
        outs() << "NOT dominate exits: " << I << "\n";
        return false;
      }
    }

    outs() << "dominate exits: " << I << "\n";
    return true;
  };

  // Check for PHIs using this value — assumes multiple defs
  auto definedOnlyOnce = [&](Instruction &I) -> bool {
    for (User *user : I.users()){  
      if (PHINode* phi = dyn_cast<PHINode>(user)){
        if (L->contains(phi)){
          outs() << "NOT defined only once: " << I << "\n";
          return false; 
        }
      }
    }
    outs() << "defined only once: " << I << "\n";
    return true;
  };

  // The instruction must be executed before all its uses
  auto dominatesAllUses = [&](Instruction &I) -> bool {
    for (User *user : I.users()) {
      if (Instruction *userInstr = dyn_cast<Instruction>(user)) {
        BasicBlock *userBlock = userInstr->getParent();
        if (userBlock && !DT.dominates(I.getParent(), userBlock)) {
          outs() << "NOT dominate all uses: " << I << "\n";
          return false;
        }
      }
    }
    outs() << "dominate all uses: " << I << "\n";
    return true;
  };

  return (isDeadOutsideLoop(I) || dominatesAllExits(I)) && definedOnlyOnce(I) && dominatesAllUses(I);
}

PassPluginLibraryInfo getLocalOptPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "LICMopt",
    "v1.0",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "LICM-opt") {
            FPM.addPass(LICMopt());
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
