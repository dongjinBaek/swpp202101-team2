#ifndef SCALETOINT64_H
#define SCALETOINT64_H

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

// Convert Int32 loads/stores to int64 loads/stores to lower the temperature.
// Malloc, alloca, and global variable sizes should be doubled.
// Int32 GetElementPtr offsets should be doubled too.
class ScaleToInt64Pass : public PassInfoMixin<ScaleToInt64Pass> {
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};

}

#endif 