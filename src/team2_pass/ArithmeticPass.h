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

/*
    Arithmetic Pass
    This pass changes some arithmetic instuctions to other instruction or constant.
    Inst. can be changed to other inst. with lower cost.
    Trivial case of arithmetic inst. can be changed to constant,
    and the use of inst. will be replaced to that constand and the inst. will be deleted.
*/

namespace backend {
class ArithmeticPass : public PassInfoMixin<ArithmeticPass> {
public:
    PreservedAnalyses run(Function &, FunctionAnalysisManager &);
};
}

#endif 