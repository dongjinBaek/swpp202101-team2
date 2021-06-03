#include "AddNoRecurseAttrs.h"

PreservedAnalyses AddNoRecurseAttrsPass::run(LazyCallGraph::SCC &C,
            CGSCCAnalysisManager &AM, LazyCallGraph &CG, CGSCCUpdateResult &) {
  // if SCC contains mutliple nodes, all these nodes have recursion
  if (C.size() != 1) return PreservedAnalyses::all();
  
  LazyCallGraph::Node &N = *C.begin();
  Function &F = N.getFunction(); // only 1 function left
  for (BasicBlock &BB : F)
    for(Instruction &I : BB.instructionsWithoutDebug())
      if (CallBase *CB = dyn_cast<CallBase>(&I)) {
        Function *Callee = CB->getCalledFunction();
        // if function points itself, it has recursion, otherwise not
        if (!Callee || Callee == &F) return PreservedAnalyses::all();
      }

  F.setDoesNotRecurse(); // set F does not recurse
  return PreservedAnalyses::none(); // setting attribute also changes analyses
}
