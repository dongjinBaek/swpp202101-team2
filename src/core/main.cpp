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

#include "llvm/Transforms/Scalar/LoopUnrollAndJamPass.h"
#include "llvm/Transforms/Scalar/LoopSink.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Scalar/DivRemPairs.h"
#include "llvm/Transforms/Scalar/SpeculateAroundPHIs.h"
#include "llvm/Transforms/IPO/ConstantMerge.h"

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

  if (shouldUsePass("SetIsNoInlinePass")) {
    MPM.addPass(buildModuleSimplificationPipeline());
    MPM.addPass(SetIsNoInlinePass());
    MPM.addPass(buildInlinerPipeline());
    MPM.addPass(buildModuleOptimizationPipeline());
  }

/*
  if (shouldUsePass("ExtractFromLoopPass")) {
    FunctionPassManager FPM;

    FPM.addPass(createFunctionToLoopPassAdaptor(std::move(LICMPass())));
    FPM.addPass(ExtractFromLoopPass());
    FPM.addPass(createFunctionToLoopPassAdaptor(std::move(LICMPass())));
    FPM.addPass(ExtractFromLoopPass());

    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
  }
*/
  if (shouldUsePass("LoopUnrollPass") || shouldUsePass("VectorizePass")) {
    MPM.addPass(RemoveLoopMetadataPass());

    FunctionPassManager OptimizePM;
    OptimizePM.addPass(LoopUnrollAndJamPass(3));
    OptimizePM.addPass(LoopUnrollPass(LoopUnrollOptions(3, false, false)
                                                   .setPartial(true)
                                                   .setRuntime(true)
                                                   .setUpperBound(true)));
    OptimizePM.addPass(InstCombinePass());
    OptimizePM.addPass(RequireAnalysisPass<OptimizationRemarkEmitterAnalysis, Function>());
    OptimizePM.addPass(createFunctionToLoopPassAdaptor(
        LICMPass(100, 250), true, true, false));
    OptimizePM.addPass(LoopSinkPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(move(OptimizePM)));
  }

  if (shouldUsePass("VectorizePass")) {
    MPM.addPass(VectorizePass());

    FunctionPassManager OptimizePM;
    OptimizePM.addPass(InstSimplifyPass());
    OptimizePM.addPass(DivRemPairsPass());
    OptimizePM.addPass(SimplifyCFGPass());
    OptimizePM.addPass(SpeculateAroundPHIsPass());
    MPM.addPass(createModuleToFunctionPassAdaptor(move(OptimizePM)));

    MPM.addPass(GlobalDCEPass());
    MPM.addPass(ConstantMergePass());
  }

  if (shouldUsePass("ScaleToInt64Pass")) {
    MPM.addPass(ScaleToInt64Pass());
  }

  if (shouldUsePass("CondBranchDeflationPass")) {
    MPM.addPass(CondBranchDeflationPass());
  }
  
  if (shouldUsePass("Malloc2DynAllocaPass")) {
    MPM.addPass(Malloc2DynAllocaPass());
  }

  if (shouldUsePass("Alloca2RegPass")) {
    MPM.addPass(Alloca2RegPass());
  }
/*
  if (shouldUsePass("IROutlinerPass")) {
    MPM.addPass(IROutlinerPass());
  }
*/
  if (shouldUsePass("ArithmeticPass")) {
    FunctionPassManager FPM2;

    FPM2.addPass(GVN());
    FPM2.addPass(IntegerEqPropagationPass());
    FPM2.addPass(GVN());
    FPM2.addPass(ArithmeticPass());
    FPM2.addPass(SimplifyCFGPass());
    
    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM2)));
  }

  MPM.run(*M, MAM);

  SplitSelfLoopPass().run(*M, MAM);
  UnfoldVectorInstPass().run(*M, MAM);
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
