#include "Pipeline.h"

using namespace llvm;
using namespace std;

FunctionPassManager buildFunctionSimplificationPipeline() {
  FunctionPassManager FPM;
  FPM.addPass(SROA());
  FPM.addPass(EarlyCSEPass(true));
  FPM.addPass(JumpThreadingPass());
  FPM.addPass(CorrelatedValuePropagationPass());
  FPM.addPass(SimplifyCFGPass());
  FPM.addPass(AggressiveInstCombinePass());
  FPM.addPass(InstCombinePass());
  FPM.addPass(TailCallElimPass());
  FPM.addPass(SimplifyCFGPass());
  FPM.addPass(ReassociatePass());
  FPM.addPass(RequireAnalysisPass<OptimizationRemarkEmitterAnalysis, Function>());

  LoopPassManager LPM1;
  LPM1.addPass(LoopInstSimplifyPass());
  LPM1.addPass(LoopSimplifyCFGPass());;
  LPM1.addPass(LoopRotatePass(true));
  LPM1.addPass(LICMPass(100, 250));
  LPM1.addPass(SimpleLoopUnswitchPass(true));
  FPM.addPass(createFunctionToLoopPassAdaptor(move(LPM1), true, true, false));
  
  FPM.addPass(SimplifyCFGPass());
  FPM.addPass(InstCombinePass());

  LoopPassManager LPM2;
  LPM2.addPass(IndVarSimplifyPass());
  LPM2.addPass(LoopDeletionPass());
  LPM2.addPass(LoopFullUnrollPass(3, false, false));
  FPM.addPass(createFunctionToLoopPassAdaptor(
      move(LPM2), false, false, false));

  FPM.addPass(SROA());
  FPM.addPass(MergedLoadStoreMotionPass());
  FPM.addPass(GVN());
  FPM.addPass(SCCPPass());
  FPM.addPass(BDCEPass());
  FPM.addPass(InstCombinePass());
  FPM.addPass(JumpThreadingPass());
  FPM.addPass(CorrelatedValuePropagationPass());
  FPM.addPass(ADCEPass());
  FPM.addPass(DSEPass());
  FPM.addPass(createFunctionToLoopPassAdaptor(
      LICMPass(100, 250), true, true, false));
  FPM.addPass(SimplifyCFGPass());
  FPM.addPass(InstCombinePass());

  return FPM;
}

ModulePassManager buildPreSimplificationPipeline() {
  ModulePassManager MPM(true);
  MPM.addPass(InferFunctionAttrsPass());

  FunctionPassManager EarlyFPM;
  EarlyFPM.addPass(SimplifyCFGPass());
  EarlyFPM.addPass(SROA());
  EarlyFPM.addPass(EarlyCSEPass());
  EarlyFPM.addPass(CallSiteSplittingPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(move(EarlyFPM)));

  MPM.addPass(IPSCCPPass());
  MPM.addPass(CalledValuePropagationPass());
  MPM.addPass(GlobalOptPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(PromotePass()));

  FunctionPassManager GlobalCleanupPM;
  GlobalCleanupPM.addPass(InstCombinePass());
  GlobalCleanupPM.addPass(SimplifyCFGPass());
  MPM.addPass(createModuleToFunctionPassAdaptor(move(GlobalCleanupPM)));

  return MPM;
}

ModuleInlinerWrapperPass buildInlinerPipeline() {
  InlineParams IP = getInlineParams(3, 0);
  ModuleInlinerWrapperPass MIWP(IP, false, true, InliningAdvisorMode::Default, 4);
  MIWP.addRequiredModuleAnalysis<GlobalsAA>();
  MIWP.addRequiredModuleAnalysis<ProfileSummaryAnalysis>();
  CGSCCPassManager &MainCGPipeline = MIWP.getPM();
  MainCGPipeline.addPass(PostOrderFunctionAttrsPass());
  MainCGPipeline.addPass(ArgumentPromotionPass());
  MainCGPipeline.addPass(createCGSCCToFunctionPassAdaptor(buildFunctionSimplificationPipeline()));
  return MIWP;
}

ModulePassManager buildPostSimplificationPipeline() {
  ModulePassManager MPM;
  MPM.addPass(GlobalOptPass());
  MPM.addPass(GlobalDCEPass());

  return MPM;
}