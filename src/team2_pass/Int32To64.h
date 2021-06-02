#ifndef INT64TO32_H
#define INT64TO32_H

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

class Int32To64Pass : public PassInfoMixin<Int32To64Pass> {
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};

}

#endif 