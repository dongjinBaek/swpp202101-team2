#ifndef CONDBR_DEFLATION_H
#define CONDBR_DEFLATION_H

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