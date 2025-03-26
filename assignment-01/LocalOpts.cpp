#include "LocalOpts.h"
using namespace llvm;

PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  bool moduleChanged = false;

  for (Function &F : M)   
    moduleChanged |= runOnFunction(F);

  return moduleChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool LocalOpts::runOnFunction(Function &F) {
  errs() << "\nRunning on function: " << F.getName() << "\n";
  bool functionChanged = false;

  for (auto &BB : F)
    functionChanged |= runOnBasicBlock(BB);

  return functionChanged;
}

bool LocalOpts::runOnBasicBlock(BasicBlock &B) {
  bool blockChanged = false;
  std::set<Instruction*> toBeErased;

  for (auto &I : B) {
    bool instructionChanged = AlgebraicIdentityOpt(I) || StrengthReductionOpt(I); 
    // MultiInstructionOpt(I);

    if (instructionChanged) 
      toBeErased.insert(&I);

    blockChanged |= instructionChanged;
  }

  for (auto *I : toBeErased) 
    I->eraseFromParent();

  errs() << (blockChanged ? "IR modified" : "Nothing modified") << "\n";
  return blockChanged;
}

bool LocalOpts::AlgebraicIdentityOpt(Instruction &I) {
  if (!I.isBinaryOp()) return false;

  // opcode -> {neutral constant value, isCommutative}
  const std::unordered_map<unsigned, std::pair<unsigned, bool>> operations = {
    {Instruction::Add, {0, true}}, 
    {Instruction::Mul, {1, true}},   
    {Instruction::Sub, {0, false}}, 
    {Instruction::SDiv, {1, false}},
    {Instruction::UDiv, {1, false}},
  };

  auto opCode = I.getOpcode();
  auto pair = operations.find(opCode);
  if (pair == operations.end()) return false; // exit if operation not supported

  auto [neutralValue, isCommutative] = pair->second;
  Value *LHS = I.getOperand(0);
  Value *RHS = I.getOperand(1);

  auto isNeutral = [neutralValue](Value *Op) -> bool {
    if (ConstantInt *CI = dyn_cast<ConstantInt>(Op))
      return CI->getValue() == neutralValue;
    return false;
  };

  // Normalize commutative operations by moving the neutral constant to RHS
  if (isCommutative && isNeutral(LHS)) std::swap(LHS, RHS);
  if (!isNeutral(RHS)) return false;

  errs() << "Applying algebraic identity: " << I << " -> " << *LHS << "\n";
  I.replaceAllUsesWith(LHS);
  return true;
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