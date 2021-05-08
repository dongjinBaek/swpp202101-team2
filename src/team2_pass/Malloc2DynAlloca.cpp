#include "Malloc2DynAlloca.h"

using namespace llvm;
using namespace std;

PreservedAnalyses Malloc2DynAllocaPass::run(Module &M, ModuleAnalysisManager &MAM) {
  IntegerType *Int64Ty = Type::getInt64Ty(M.getContext());

  // Define i8* $dynamic_alloca(i64), i64 @$sp.
  FunctionCallee Func = M.getOrInsertFunction("$dyn_alloca",
    Type::getInt8PtrTy(M.getContext()), Int64Ty, Int64Ty);
  Constant *sp = M.getOrInsertGlobal("$sp", Int64Ty);

  // Replace malloc meeting conditions with dynamic_alloca.
  bool replaced = false;
  for (Function &F : M)
    if (F.getName() == "main") { // Function should be "main".
      DominatorTree DT(F);
      LoopInfo LI(DT);
      for (BasicBlock &BB : F)
        if(!LI.getLoopFor(&BB)) // BasicBlock should not be in loop.
          for (auto it = BB.begin(), end = BB.end(); it != end;) {
            Instruction &I = *it++;
            CallInst *CI = dyn_cast<CallInst>(&I);
            if (CI && CI->getCalledFunction()->getName() == "malloc") {
              IRBuilder<> IRB(CI);
              LoadInst *LI = IRB.CreateLoad(sp);
              Value *Args[] = {CI->getArgOperand(0), LI};
              CallInst *newCI = CallInst::Create(Func, ArrayRef<Value *>(Args, 2));
              ReplaceInstWithInst(CI, newCI);
              replaced = true;
            }
          }
    }
  
  // If there is a replacement, add conditional branch to check if address
  // space is in stack or not.
  if (replaced) { 
    // Find all free() and collect them into vector.
    vector<CallInst *> CIs;
    for (Function &F : M)
      for (BasicBlock &BB : F)
        for (Instruction &I : BB) {
          CallInst *CI = dyn_cast<CallInst>(&I);
          if (CI && CI->getCalledFunction()->getName() == "free")
            CIs.emplace_back(CI);
        }

    // Insert conditional branches right before free().
    for (CallInst *CI : CIs) {
      BasicBlock *BB = CI->getParent();
      IRBuilder<> IRB(CI);
      Value *Addr = IRB.CreatePtrToInt(CI->getArgOperand(0), Int64Ty);
      Value *Cond = IRB.CreateICmpULE(Addr, ConstantInt::get(Int64Ty, 102400));
      // Make conditional branch.
      Instruction *ThenTerm = SplitBlockAndInsertIfThen(Cond, CI, false);
      CI->moveBefore(ThenTerm); // Move free() from tail to then block.
      BranchInst *HeadTerm = dyn_cast<BranchInst>(BB->getTerminator());
      HeadTerm->swapSuccessors(); // Swap then and tail block.
    }
  }

  return PreservedAnalyses::all();
}
