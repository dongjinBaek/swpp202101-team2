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
#include "../team2_pass/Alloca2RegPass.h"
#include "../team2_pass/RemoveLoopMetadata.h"
#include "../team2_pass/Vectorize.h"
#include "../team2_pass/ExtractFromLoopPass.h"
#include "../team2_pass/IROutliner.h"
#include "../team2_pass/SetIsNoInline.h"
#include "../team2_pass/Pipeline.h"
#include "../team2_pass/ScaleToInt64.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/IR/PassManager.h"

#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"

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
  ModulePassManager MPM;

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

  MPM.addPass(RemoveLoopMetadataPass());

  MPM.addPass(buildModuleSimplificationPipeline());
  CGSCCPassManager CPM;
  CPM.addPass(createCGSCCToFunctionPassAdaptor(
      buildFunctionSimplificationPipeline()));
  MPM.addPass(createModuleToPostOrderCGSCCPassAdaptor(move(CPM)));

  MPM.addPass(SetIsNoInlinePass());
  MPM.addPass(buildInlinerPipeline());

  MPM.addPass(buildModuleOptimizationPipeline());

  // load/store vectorize
  MPM.addPass(VectorizePass());

  FunctionPassManager GlobalCleanupPM;
  GlobalCleanupPM.addPass(InstCombinePass());
  GlobalCleanupPM.addPass(SimplifyCFGPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(move(GlobalCleanupPM)));

  MPM.addPass(buildModulePostOptimizationPipeline());

  // scale to int 64 to relieve temperature
  MPM.addPass(ScaleToInt64Pass());

  // conditional branch true <-> false, branch to switch, etc.
  MPM.addPass(CondBranchDeflationPass());

  // memory related optimizations
  MPM.addPass(Malloc2DynAllocaPass());
  MPM.addPass(Alloca2RegPass());

  //MPM.addPass(IROutlinerPass());

  FunctionPassManager FPM;
  FPM.addPass(IntegerEqPropagationPass());
  FPM.addPass(GVN());
  FPM.addPass(ArithmeticPass());
  FPM.addPass(GVN());
  FPM.addPass(SimplifyCFGPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(move(FPM)));

  MPM.run(*M, MAM);

  SplitSelfLoopPass().run(*M, MAM);
  UnfoldVectorInstPass().run(*M, MAM);
  AddressArgCastPass().run(*M, MAM);
  ConstExprRemovePass().run(*M, MAM);
  GEPUnpackPass().run(*M, MAM);
  RegisterSpillPass().run(*M, MAM);

  // use this for debugging
  // outs() << *M;

  // execute backend to emit assembly
  Backend B(optOutput, optPrintProgress);
  B.run(*M, MAM);
  
  return 0;
}
