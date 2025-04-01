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
    functionChanged |= runOnBasicBlock(BB);

  return functionChanged;
}

bool LocalOpts::runOnBasicBlock(BasicBlock &B) {
  bool blockChanged = false;
  std::set<Instruction*> toBeErased;

  for (auto &I : B) {
    bool instructionChanged = 
      I.isBinaryOp() &&
      AlgebraicIdentityOpt(I) || 
      MultiInstructionOpt(I) ||
      SubMultiInstrOpt(I) ||
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
  if (pair == operations.end()) return false; // Unsupported operation

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
  if (pair == operations.end()) return false; // Unsupported operation

  auto ShiftOp = pair->second;
  Value *LHS = I.getOperand(0);
  Value *RHS = I.getOperand(1);

  auto isConstPowOf2 = [](Value *op) {
    if (auto *CI = dyn_cast<ConstantInt>(op)) 
      return CI->getValue().isPowerOf2();
    return false;
  };

  // Normalize commutative operations by moving the constant to RHS
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

// Handles advSR x * 15 → (x << 4) - x.
bool LocalOpts::AdvancedMulSROpt(Instruction &I) {
  auto opCode = I.getOpcode();
  if (opCode != Instruction::Mul) return false; 

  Value *LHS = I.getOperand(0);
  Value *RHS = I.getOperand(1);
  ConstantInt *CI = nullptr;
  
  // Ensure the constant is on RHS
  if ((CI = dyn_cast<ConstantInt>(LHS))) std::swap(LHS, RHS);
  if (!(CI = dyn_cast<ConstantInt>(RHS)) || CI->getValue().isZero()) return false; 

  Instruction::BinaryOps adjustOp;
  unsigned ShiftValue;
  bool isValid = false;

  if ((CI->getValue()+1).isPowerOf2()) {
    adjustOp = Instruction::Sub;
    ShiftValue = (CI->getValue()+1).logBase2();
    isValid = true;
  }

  if ((CI->getValue()-1).isPowerOf2()) {
    adjustOp = Instruction::Add;
    ShiftValue = (CI->getValue()-1).logBase2();
    isValid = true;
  } 

  if (!isValid) return false;

  auto *ShiftInstr = BinaryOperator::Create(Instruction::Shl, LHS, ConstantInt::get(CI->getType(), ShiftValue));
  auto *AdjustInstr = BinaryOperator::Create(adjustOp, ShiftInstr, LHS);

  errs() << "Adv Strength Reduction: " << I << "\n";
  ShiftInstr->insertBefore(&I);
  AdjustInstr->insertAfter(ShiftInstr);
  I.replaceAllUsesWith(AdjustInstr);
  return true;
}

bool LocalOpts::MultiInstructionOpt(Instruction &I) {
  // opcode -> opposite operation
  const std::unordered_map<unsigned, Instruction::BinaryOps> operations = {
    {Instruction::Add, Instruction::Sub},
    {Instruction::Sub, Instruction::Add},
  };

  auto opCode = I.getOpcode();
  auto pair = operations.find(opCode);
  if (pair == operations.end()) return false; // Unsupported operation

  auto InverseOp = pair->second;

  // Extract operands and normalize constants to RHS
  auto getOperands = [](Instruction &I) -> std::pair<Value*, ConstantInt*> {
    Value *LHS = I.getOperand(0);
    Value *RHS = I.getOperand(1);
    ConstantInt *CI = nullptr;

    if (I.getOpcode() == Instruction::Add && (CI = dyn_cast<ConstantInt>(LHS))) std::swap(LHS, RHS);
    CI = dyn_cast<ConstantInt>(RHS);

    return {LHS, CI};
  }; 

  auto operandsPair1 = getOperands(I);
  auto *UsedInstr = dyn_cast<Instruction>(operandsPair1.first);
  if (!UsedInstr || !UsedInstr->isBinaryOp()) return false;

  auto operandsPair2 = getOperands(*UsedInstr);
  auto opCode2 = UsedInstr->getOpcode();

  if (opCode2 != InverseOp || operandsPair2.second != operandsPair1.second) return false;

  errs() << "Multi Instruction: " << I << "\n";
  I.replaceAllUsesWith(operandsPair2.first);
  return true;
}

// Subtraction-based multi-instr patterns, e.g., a = 1 - b, c = 1 - a → c = b.
bool LocalOpts::SubMultiInstrOpt(Instruction &I){
  auto opCode = I.getOpcode();
  if (opCode != Instruction::Sub) return false; 

  Value *LHS = I.getOperand(0);
  Value *RHS = I.getOperand(1);
  ConstantInt *CI = nullptr;
  Instruction *UsedInstr = nullptr;

  if (!(CI = dyn_cast<ConstantInt>(LHS))) return false;
  if (!(UsedInstr = dyn_cast<Instruction>(RHS))) return false;

  auto opCode2 = UsedInstr->getOpcode();
  if (opCode2 != Instruction::Sub) return false; 

  Value *LHS2 = UsedInstr->getOperand(0);
  Value *RHS2 = UsedInstr->getOperand(1);
  ConstantInt *CI2 = nullptr;

  if (!(CI2 = dyn_cast<ConstantInt>(LHS2))) return false;
  if (CI != CI2) return false;

  errs() << "Multi Instruction Sub: " << I << "\n";
  I.replaceAllUsesWith(RHS2);
  return true;
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