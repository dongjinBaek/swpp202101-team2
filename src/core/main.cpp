#include "../backend/Backend.h"
#include "../backend/AddressArgCast.h"
#include "../backend/ConstExprRemove.h"
#include "../backend/GEPUnpack.h"
#include "../backend/RegisterSpill.h"
#include "../backend/UnfoldVectorInst.h"
#include "../backend/SplitSelfLoop.h"

#include "../team2_pass/CondBranchDeflation.h"
#include "../team2_pass/ArithmeticPass.h"
#include "../team2_pass/IntegerEqPropagation.h"
#include "../team2_pass/Malloc2DynAlloca.h"
#include "../team2_pass/IROutliner.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/CorrelatedValuePropagation.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"

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

static cl::OptionCategory optCategory("SWPP Compiler options");

static cl::opt<string> optInput(
    cl::Positional, cl::desc("<input bitcode file>"), cl::Required,
    cl::value_desc("filename"), cl::cat(optCategory));

static cl::opt<string> optOutput(
    cl::Positional, cl::desc("output assembly file"), cl::cat(optCategory),
    cl::init("a.s"));

static cl::list<string> optUsePass("p", cl::desc("used passes"), cl::cat(optCategory));

bool shouldUsePass(string arg)
{
  if (optUsePass.size() == 0) {
    return true;
  }
  for (unsigned i=0; i < optUsePass.size(); ++i) {
    if (arg == optUsePass[i]) {
      return true;
    }
  }
  return false;
}

int main(int argc, char *argv[]) {
  cl::ParseCommandLineOptions(argc, argv);
  //Parse command line arguments
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

  if (shouldUsePass("InlinerPass")) {
    CPM.addPass(InlinerPass());
    MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(std::move(CPM)));
  }
  if (shouldUsePass("Malloc2DynAllocaPass")) {
    // malloc 2 alloca passes
    MPM.addPass(Malloc2DynAllocaPass());
  }
  
  if (shouldUsePass("CondBranchDeflationPass")) {
    // cond branch pass
    MPM.addPass(CondBranchDeflationPass());
  }
  
  if (shouldUsePass("LoopUnrollPass")) {
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
  }
  
  if (shouldUsePass("SimplifyPasses")) {
    // // simplify passes
    FPM.addPass(SimplifyCFGPass());
    FPM.addPass(InstCombinePass());
  }
    
  if (shouldUsePass("ArithmeticPass")) {
    // arithmetic passes
    FPM.addPass(GVN());
    FPM.addPass(IntegerEqPropagationPass());
    FPM.addPass(GVN());
    FPM.addPass(ArithmeticPass());
    FPM.addPass(SimplifyCFGPass());
  }

  if (shouldUsePass("IROutlinerPass")) {
    MPM.addPass(IROutlinerPass());
  }
  
  MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  
  MPM.run(*M, MAM);
  
  SplitSelfLoopPass().run(*M, MAM);
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
