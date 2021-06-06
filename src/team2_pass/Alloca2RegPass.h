#ifndef ALLOCA_TO_REG_H
#define ALLOCA_TO_REG_H

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

#include <string>
#include <vector>

using namespace llvm;
using namespace std;

/*
    Alloca2RegPass
*/

namespace team2_pass {
class Alloca2RegPass : public PassInfoMixin<Alloca2RegPass> {
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
private:
    uint64_t n;
    int elementBitWidth;
    vector<LoadInst *> Regs;
    vector<Instruction *> InstsToRemove;
    void changeUseOfGEPToSwitch(GetElementPtrInst *, Function &,FunctionCallee &);
    bool canChangeAlloca2Reg(AllocaInst *);
};
}

#endif 