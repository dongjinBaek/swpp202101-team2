#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

class MallocInlinerPass : public PassInfoMixin<MallocInlinerPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};
