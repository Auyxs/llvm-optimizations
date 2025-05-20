#ifndef LOOPFUSION_OPT_H
#define LOOPFUSION_OPT_H

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"

#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"

namespace llvm {

class LoopFusionOpt : public PassInfoMixin<LoopFusionOpt> {
    public:
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
        bool runOnLoops(Function &F, FunctionAnalysisManager &FAM, const std::vector<Loop*> &Loops);
        bool isOptimizable(Function &F, FunctionAnalysisManager &FAM, Loop *prev, Loop *curr);
        Loop* optimizeLoop(Function &F, FunctionAnalysisManager &FAM, Loop *prev, Loop *curr);
        
    };
}

#endif