#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace std;

class Malloc2DynAllocaPass : public PassInfoMixin<Malloc2DynAllocaPass> {
public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};
