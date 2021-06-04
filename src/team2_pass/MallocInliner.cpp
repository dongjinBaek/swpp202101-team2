#include "MallocInliner.h"

using namespace llvm;
using namespace std;

static void updateList(Function &F, map<Function *, bool> &list);

PreservedAnalyses MallocInlinerPass::run(Module &M, ModuleAnalysisManager &MAM) {
  // find non-recursive functions supported by PostOrderFunctionAttrsPass
  // if f1 -> f2 -> f3 -> f1, then all these three functions are recursive
  // main is by definition not recursive
  // among them, find all functions having no-loop malloc
  // having something means, if f1 -> f2, then f1 has f2
  map<Function *, bool> list; // no malloc functions inside list
  // non: recursive, true: no recursion no loop, false: no recursion but loop
  // malloc doesNotRecurse, function with exact definition has chance to set it
  for (Function &F : M)
    if (F.doesNotRecurse() && list.find(&F) == list.end())
      updateList(F, list); // update to true or false

  // if main has no-loop malloc, inline occurs
  // inline functions having no-loop malloc in main until no update
  Function *main = M.getFunction("main");
  if (!main || list.find(main) == list.end() || !list[main])
    return PreservedAnalyses::all(); // list[main] false -> no change to inline

  bool update = true;
  while(update) {
    update = false;
    DominatorTree DT(*main); // renew whenever update occurs
    LoopInfo LI(DT); // renew whenever update occurs
    for (BasicBlock &BB : *main) {
      if (!LI.getLoopFor(&BB)) // no loop
        for (Instruction &I : BB) {
          CallInst *CI = dyn_cast<CallInst>(&I);
          if(CI) {
            Function *CF = CI->getCalledFunction();
            if (list.find(CF) != list.end() && list[CF]) { // can do inline
              InlineFunctionInfo IFI;
              InlineResult res = InlineFunction(*CI, IFI);
              assert(res.isSuccess() && "unexpected failure to inline");
              update = true;
              break; // update step by step
            }
          }
        }
      if (update) break;
    }
  }

  return PreservedAnalyses::none();
}

static void updateList(Function &F, map<Function *, bool> &list) {
  DominatorTree DT(F);
  LoopInfo LI(DT);

  for (BasicBlock &BB : F)
    if (!LI.getLoopFor(&BB)) // no loop
      for (Instruction &I : BB) {
        CallInst *CI = dyn_cast<CallInst>(&I);
        if (CI) {
          Function *CF = CI->getCalledFunction();
          if (CF->getName() == "malloc") { // malloc
            list[&F] = true;
            return;
          }
          else if (CF->doesNotRecurse()) { // no recursion
            if (list.find(CF) == list.end())
              updateList(*CF, list);
            if (list[CF]) {
              list[&F] = true;
              return;
            }
          }
        }
      }
  list[&F] = false;
}
