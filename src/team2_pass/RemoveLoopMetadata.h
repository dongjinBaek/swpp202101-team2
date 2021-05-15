#ifndef REMOVE_METADATA_H
#define REMOVE_METADATA_H

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

namespace team2_pass {

// to enable loop unrolling with removing 'llvm.loop.unroll.disable' metadata,
// remove all loop metadata.
class RemoveLoopMetadataPass : public PassInfoMixin<RemoveLoopMetadataPass> {
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};
}

#endif 