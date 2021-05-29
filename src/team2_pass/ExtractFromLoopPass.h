#ifndef EXTRACT_FROM_LOOP_H
#define EXTRACT_FROM_LOOP_H

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
#include <string>

using namespace llvm;
using namespace std;

/*
    ExtractFromLoopPass
*/

namespace backend {
class ExtractFromLoopPass : public PassInfoMixin<ExtractFromLoopPass> {
public:
    PreservedAnalyses run(Function &, FunctionAnalysisManager &);
private:
 void extractFromLoop(Loop *loop, DominatorTree &DT);
};
}

#endif 