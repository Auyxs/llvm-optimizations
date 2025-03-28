#include "LocalOpts.h"
using namespace llvm;

PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  bool moduleChanged = false;

  for (Function &F : M)   
    moduleChanged |= runOnFunction(F);

  errs() << (moduleChanged ? "IR modified" : "Nothing modified") << "\n";
  return moduleChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool LocalOpts::runOnFunction(Function &F) {
  errs() << "\nRunning on function: " << F.getName() << "\n";
  bool functionChanged = false;

  for (auto &BB : F)
    functionChanged |= 
      runOnBasicBlock(BB) || 
      MultiInstructionOpt(BB);

  return functionChanged;
}

bool LocalOpts::runOnBasicBlock(BasicBlock &B) {
  bool blockChanged = false;
  std::set<Instruction*> toBeErased;

  for (auto &I : B) {
    bool instructionChanged = 
      I.isBinaryOp() &&
      AlgebraicIdentityOpt(I) || 
      StrengthReductionOpt(I) ||
      AdvancedMulSROpt(I); 
    
    if (instructionChanged) 
      toBeErased.insert(&I);

    blockChanged |= instructionChanged;
  }

  for (auto *I : toBeErased) 
    I->eraseFromParent();

  return blockChanged;
}

bool LocalOpts::AlgebraicIdentityOpt(Instruction &I) {
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

  auto neutralValue = pair->second.first; 
  auto isCommutative = pair->second.second;
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

  errs() << "Algebraic Identity: " << I << "\n";
  I.replaceAllUsesWith(LHS);
  return true;
}

bool LocalOpts::StrengthReductionOpt(Instruction &I) {
  // opcode -> shift operation
  const std::unordered_map<unsigned, Instruction::BinaryOps> operations = {
    {Instruction::Mul, Instruction::Shl},
    {Instruction::UDiv, Instruction::LShr},
    {Instruction::SDiv, Instruction::AShr}
  };

  auto opCode = I.getOpcode();
  auto pair = operations.find(opCode);
  if (pair == operations.end()) return false; // exit if operation not supported

  auto ShiftOp = pair->second;
  Value *LHS = I.getOperand(0);
  Value *RHS = I.getOperand(1);

  auto isConstPowOf2 = [](Value *op) {
    if (auto *CI = dyn_cast<ConstantInt>(op)) 
      return CI->getValue().isPowerOf2();
    return false;
  };

  // Normalize commutative operations by moving the neutral constant to RHS
  if (opCode == Instruction::Mul && isConstPowOf2(LHS)) std::swap(LHS, RHS); 
  if (!isConstPowOf2(RHS)) return false;

  auto *CI = dyn_cast<ConstantInt>(RHS); 
  unsigned ShiftValue = CI->getValue().logBase2();
  auto *ShiftInstr = BinaryOperator::Create(ShiftOp, LHS, ConstantInt::get(CI->getType(), ShiftValue));

  errs() << "Strength Reduction: " << I << "\n";
  ShiftInstr->insertBefore(&I);
  I.replaceAllUsesWith(ShiftInstr);
  return true;
}

bool LocalOpts::AdvancedMulSROpt(Instruction &I) {
  auto opCode = I.getOpcode();
  if (opCode != Instruction::Mul) return false; 

  Value *LHS = I.getOperand(0);
  Value *RHS = I.getOperand(1);

  // ensure the neutral constant is on RHS
  ConstantInt *CI = dyn_cast<ConstantInt>(LHS);
  if (CI) std::swap(LHS, RHS);
  CI = dyn_cast<ConstantInt>(RHS);
  if (!CI || CI->getValue().isZero()) return false; 

  Instruction::BinaryOps adjustOp;
  unsigned ShiftValue;

  if ((CI->getValue()+1).isPowerOf2()) {
    adjustOp = Instruction::Sub;
    ShiftValue = (CI->getValue()+1).logBase2();
  }

  if ((CI->getValue()-1).isPowerOf2()) {
    adjustOp = Instruction::Add;
    ShiftValue = (CI->getValue()-1).logBase2();
  } 

  auto *ShiftInstr = BinaryOperator::Create(Instruction::Mul, LHS, ConstantInt::get(CI->getType(), ShiftValue));
  auto *AdjustInstr = BinaryOperator::Create(adjustOp, ShiftInstr, LHS);

  errs() << "Adv Strength Reduction: " << I << "\n";
  ShiftInstr->insertBefore(&I);
  AdjustInstr->insertAfter(ShiftInstr);
  I.replaceAllUsesWith(AdjustInstr);
  return true;
}

bool LocalOpts::MultiInstructionOpt(BasicBlock &B) {
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