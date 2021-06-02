#ifndef VECTORIZE_H
#define VECTORIZE_H

#include <algorithm>
#include <map>
#include <utility>

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace std;

namespace team2_pass {

// type for integer difference of two value
typedef struct {
    bool known;
    int64_t value;
} Difference;

// Vectorize consecutive load and store instructions.
// This pass should come after LoopUnrollPass and SimplifyCFG.
class VectorizePass : public PassInfoMixin<VectorizePass> {
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
    static Difference getDifference(Value *V1, Value *V2);
    
private:
    Type *VoidTy;
    Type *Int64Ty;
    Type *Int64PtrTy;
    map<int, Type *> VectIntTypes;
    map<int, FunctionCallee> VLoads;
    map<int, FunctionCallee> VStores;
    map<int, FunctionCallee> ExtractElements;

    void Vectorize(SmallVector<Instruction *, 8> &, SmallVector<int, 8> &, bool);
    Instruction *findNextBaseInstruction(Instruction *I);
    void runOnBasicBlock(BasicBlock &BB);
    void declareFunctions(Module &M);
    void sinkAllLoadUsers(BasicBlock &BB);
    void sinkRecursive(BasicBlock &BB, Instruction *I);
    Instruction *findNextMemoryInstruction(Instruction *I);
    bool doesAccessMemory(CallInst *CI);
};

}

#endif 