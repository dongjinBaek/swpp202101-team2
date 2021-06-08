#include "llvm/Passes/PassBuilder.h"
#include "../backend/LivenessAnalysis.h"
#include "SetNoRecursion.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

class SetIsNoInlinePass : public PassInfoMixin<SetIsNoInlinePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
  static const int THRESHOLD = 32;
};
