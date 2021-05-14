#ifndef INTEGER_EQ_PROPAGATE_H
#define INTEGER_EQ_PROPAGATE_H

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

using namespace llvm;
using namespace std;

namespace backend {
class IntegerEqPropagationPass : public PassInfoMixin<IntegerEqPropagationPass> {
public:
    PreservedAnalyses run(Function &, FunctionAnalysisManager &);

private:
    bool changeUseIfEdgeDominates(Value *ChangeFrom, Value *ChangeTo, DominatorTree &DT, BasicBlockEdge &BBE);
};
}

#endif 