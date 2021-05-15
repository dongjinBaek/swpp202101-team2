#ifndef CONDBR_DEFLATION_H
#define CONDBR_DEFLATION_H

/*******************************************************************************************
CondBranchDeflationPass

* Attempt to reduce cost incurred by conditional branches.
* If a block(BasicBlock) terminates with a conditional branch
whose condition is an ICmp with no other use
and whose true edge is repeatable while the false edge is not,
(e.g. is part of a cycle(loop), leads to a block containing recursive call, etc.)
invert the condition and swap its successor blocks;
* if both edges are repeatable or neither,
replace the branch with an equivalent switch instruction.

*******************************************************************************************/

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include<vector>
#include<algorithm>

using namespace llvm;
using namespace std;

namespace team2_pass {
class CondBranchDeflationPass : public PassInfoMixin<CondBranchDeflationPass> {
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};

extern vector<StringRef> v;
}

#endif 