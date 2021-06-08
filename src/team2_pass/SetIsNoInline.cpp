#include "SetIsNoInline.h"

using namespace llvm;
using namespace std;

static bool visitFunction(Function &F, set<Function *> &Visit,
                    set<Function *> &NRS, map<CallBase *, CallBase *> &CB2CCB);

PreservedAnalyses SetIsNoInlinePass::run(Module &M, ModuleAnalysisManager &MAM) {
  vector<CallBase *> CBs;
  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    for (BasicBlock &BB : F) for (Instruction &I : BB)
      if (CallBase *CB = dyn_cast<CallBase>(&I)) {
        Function *Callee = CB->getCalledFunction();
        if (!Callee || Callee->isDeclaration()) continue;
        CBs.push_back(CB); // collect all CallBases
      }
  }
  auto CM = CloneModule(M); // clone module for testing inlining
  vector<CallBase *> CCBs;
  for (Function &F : *CM) {
    if (F.isDeclaration()) continue;
    for (BasicBlock &BB : F) for (Instruction &I : BB)
      if (CallBase *CB = dyn_cast<CallBase>(&I)) {
        Function *Callee = CB->getCalledFunction();
        if (!Callee || Callee->isDeclaration()) continue;
        CCBs.push_back(CB); // collect all CallBases of cloned module
      }
  }
  assert(CBs.size() == CCBs.size() && "number of CallBases should be the same");
  int size = CBs.size();
  map<CallBase *, CallBase *> CB2CCB;
  for (int i = 0; i < size; i++)
    CB2CCB[CBs[i]] = CCBs[i]; // do mapping, CB <-> Cloned CB

  set<Function *> NRS; // no recursion set
  ModulePassManager MPM;
  MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(
      SetNoRecursionPass(&NRS)));
  MPM.run(M, MAM); // find no recursion functions in cloned module

  set<Function *> Visit;
  bool Changed = false;
  for (Function &F : M) {
    if (F.isDeclaration() || Visit.find(&F) != Visit.end()) continue;
    Changed |= visitFunction(F, Visit, NRS, CB2CCB); // F can be recursive
  }
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

static bool visitFunction(Function &F, set<Function *> &Visit,
                    set<Function *> &NRS, map<CallBase *, CallBase *> &CB2CCB) {
  Visit.insert(&F); // check visit
  bool Changed = false;

  for (BasicBlock &BB : F)
    for (Instruction &I : BB)
      if (CallBase *CB = dyn_cast<CallBase>(&I)) {
        Function *Callee = CB->getCalledFunction();
        if (!Callee || Callee->isDeclaration() || NRS.find(Callee) == NRS.end())
          continue; // Callee is not recursive and has definition

        if (Visit.find(Callee) == Visit.end()) // bottom-up update
          Changed |= visitFunction(*Callee, Visit, NRS, CB2CCB);

        CallBase *CCB = CB2CCB[CB];
        // every time RegisterGraph should be newly created
        backend::RegisterGraph RG(*CCB->getModule());
        int NumColors = RG.getNumColors(CCB->getFunction()) +
                        RG.getNumColors(CCB->getCalledFunction());
        // heuristic: actual number of colors after inlining would be smaller
        // or equal than NumColors
        if (NumColors > SetIsNoInlinePass::THRESHOLD) {
          CB->setIsNoInline();
          Changed = true;
        }
        else {
          InlineFunctionInfo IFI;
          InlineFunction(*CCB, IFI); // actually do inline in cloned module
        }
      }
  return Changed;
}
