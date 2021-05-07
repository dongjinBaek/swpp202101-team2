#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/PatternMatch.h"

#include "llvm/Analysis/PostDominators.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

using namespace llvm;
using namespace std;

namespace backend {
class ArithmeticPass : public PassInfoMixin<ArithmeticPass> {
public:
    PreservedAnalyses run(Function &, FunctionAnalysisManager &);

private:
    void propIntEq(Function &, FunctionAnalysisManager &);
    void changeUseIfEdgeDominates(Value *ChangeFrom, Value *ChangeTo, DominatorTree &DT, BasicBlockEdge &BBE);
};
}

#endif 