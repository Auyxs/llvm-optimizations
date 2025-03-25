#include "LocalOpts.h"
using namespace llvm;

PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  bool modified = false;

  for (Function &F : M)   
    modified |= runOnFunction(F);

  return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool LocalOpts::runOnFunction(Function &F) {
  errs() << "\nRunning on function: " << F.getName() << "\n";
  bool Transformed = false;

  for (auto &BB : F)
    Transformed |= runOnBasicBlock(BB);

  return Transformed;
}

bool LocalOpts::runOnBasicBlock(BasicBlock &B) {
  bool globallyModified = false;
  std::set<Instruction*> toBeErased;

  for (auto &I : B) {
    bool locallyModified = 
      AlgebraicIdentityOpt(I) ||
      StrengthReductionOpt(I) ||
      MultiInstructionOpt(I);

    if (locallyModified) 
      toBeErased.insert(&I);
    globallyModified |= locallyModified;
  }

  if (globallyModified)
    for (auto *I : toBeErased) 
      I->eraseFromParent();

  errs() << (globallyModified ? "IR modified" : "Nothing modified") << "\n";
  return globallyModified;
}

bool LocalOpts::AlgebraicIdentityOpt(Instruction &I) {
    return false;
}

bool LocalOpts::StrengthReductionOpt(Instruction &I) {
    return false;
}

bool LocalOpts::MultiInstructionOpt(Instruction &I) {
    return false;
}

PassPluginLibraryInfo getLocalOptPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "LocalOpts",
    "v1.0",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, ModulePassManager &MPM,
          ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "local-opts") {
            MPM.addPass(LocalOpts());
            return true;
          }
          return false;
      });
    }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getLocalOptPluginInfo();
}