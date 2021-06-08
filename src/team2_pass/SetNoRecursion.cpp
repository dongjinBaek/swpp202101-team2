#include "SetNoRecursion.h"

SetNoRecursionPass::SetNoRecursionPass(set<Function *> *PassNoRecursionSet)
    : NoRecursionSet(PassNoRecursionSet) {
  NoRecursionSet->clear();
}

PreservedAnalyses SetNoRecursionPass::run(LazyCallGraph::SCC &C,
          CGSCCAnalysisManager &AM, LazyCallGraph &CG, CGSCCUpdateResult &UR) {
  // if SCC contains multiple nodes, all these nodes have recursion
  if (C.size() != 1) return PreservedAnalyses::all();

  LazyCallGraph::Node &N = *C.begin();
  Function &F = N.getFunction(); // 1 function left, which is not a declaration
  for (BasicBlock &BB : F)
    for (Instruction &I : BB.instructionsWithoutDebug())
      if (CallBase *CB = dyn_cast<CallBase>(&I)) {
        Function *Callee = CB->getCalledFunction();
        // if function points itself, it has recursion, otherwise not
        if (Callee && Callee == &F) return PreservedAnalyses::all();
      }
  NoRecursionSet->insert(&F); // set F as a function having no recursion
  return PreservedAnalyses::all();
}
