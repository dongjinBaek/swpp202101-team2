#include "../backend/Backend.h"
#include "../backend/AddressArgCast.h"
#include "../backend/ConstExprRemove.h"
#include "../backend/GEPUnpack.h"
#include "../backend/RegisterSpill.h"
#include "../backend/UnfoldVectorInst.h"

#include "../team2_pass/CondBranchDeflation.h"
#include "../team2_pass/ArithmeticPass.h"
#include "../team2_pass/IntegerEqPropagation.h"
#include "../team2_pass/Malloc2DynAlloca.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/CorrelatedValuePropagation.h"

#include "llvm/Support/Debug.h"

#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

#include "llvm/Transforms/Scalar/LoopInstSimplify.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Scalar/LoopRotation.h"
#include "llvm/Transforms/Scalar/LoopSimplifyCFG.h"
#include "llvm/Transforms/Scalar/LoopUnrollPass.h"

#include "../team2_pass/RemoveLoopMetadata.h"

#include <string>

using namespace std;
using namespace llvm;
using namespace team2_pass;

int main(int argc, char *argv[]) {
  //Parse command line arguments
  if(argc!=3) return -1;
  string optInput = argv[1];
  string optOutput = argv[2];
  bool optPrintProgress = false;

  //Parse input LLVM IR module
  LLVMContext Context;
  unique_ptr<Module> M;  

  SMDiagnostic Error;
  M = parseAssemblyFile(optInput, Error, Context);

  //If loading file failed:
  string errMsg;
  raw_string_ostream os(errMsg);
  Error.print("", os);

  if (!M)
    return 1;

  // execute IR passes
  LoopPassManager LPM;
  FunctionPassManager FPM;
  ModulePassManager MPM;
  CGSCCPassManager CPM;

  LoopAnalysisManager LAM;
  FunctionAnalysisManager FAM;
  CGSCCAnalysisManager CGAM;
  ModuleAnalysisManager MAM;

  PassBuilder PB;


  // register all the basic analyses with the managers.
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  // malloc 2 alloca passes
  CPM.addPass(InlinerPass());
  MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(std::move(CPM)));
  MPM.addPass(Malloc2DynAllocaPass());
  
  // cond branch pass
  MPM.addPass(CondBranchDeflationPass());
   
  // loop passes
  MPM.addPass(RemoveLoopMetadataPass());
  
  LPM.addPass(LoopInstSimplifyPass());
  LPM.addPass(LoopSimplifyCFGPass());
  LPM.addPass(LoopRotatePass());

  FPM.addPass(createFunctionToLoopPassAdaptor(std::move(LPM)));
  FPM.addPass(LoopUnrollPass(LoopUnrollOptions().setPartial(true)
                                                .setPeeling(true)
                                                .setProfileBasedPeeling(true)
                                                .setRuntime(true)
                                                .setUpperBound(true)));
  
  // simplify passes
  FPM.addPass(SimplifyCFGPass());
  FPM.addPass(InstCombinePass());
  
  // arithmetic passes
  FPM.addPass(GVN());
  FPM.addPass(IntegerEqPropagationPass());
  FPM.addPass(GVN());
  FPM.addPass(ArithmeticPass());
  FPM.addPass(SimplifyCFGPass());
  
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  
  MPM.run(*M, MAM);
  
  UnfoldVectorInstPass().run(*M, MAM);
  LivenessAnalysis().run(*M, MAM);
  SpillCostAnalysis().run(*M, MAM);
  AddressArgCastPass().run(*M, MAM);
  ConstExprRemovePass().run(*M, MAM);
  GEPUnpackPass().run(*M, MAM);
  RegisterSpillPass().run(*M, MAM);

  // use this for debugging
  outs() << *M;

  // execute backend to emit assembly
  Backend B(optOutput, optPrintProgress);
  B.run(*M, MAM);
  
  return 0;
}
