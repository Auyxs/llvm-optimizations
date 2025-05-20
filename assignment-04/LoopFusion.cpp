#include "LoopFusion.h"
using namespace llvm;

PreservedAnalyses LoopFusionOpt::run(Function &F, FunctionAnalysisManager &FAM) {
  outs() << "\nRunning LoopFusionOpt on function: " << F.getName() << "\n";

  auto &LI = FAM.getResult<LoopAnalysis>(F);
  bool changed = false;

  const std::vector<Loop*>& topLevelLoops = LI.getTopLevelLoops();
  changed |= runOnLoops(F, FAM, topLevelLoops);

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

Loop* LoopFusionOpt::optimizeLoop(Function &F, FunctionAnalysisManager &FAM, Loop *prev, Loop *curr){
  return curr;
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
