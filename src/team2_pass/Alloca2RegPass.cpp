/*
    Alloca2RegPass
*/

#include "Alloca2RegPass.h"

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace team2_pass {

PreservedAnalyses Alloca2RegPass::run(Module &M, ModuleAnalysisManager &MAM) {
            Regs = vector<LoadInst *>();
  for (auto &F : M) {
    int cnt = 0;
    for (auto &BB : F) {
      for (auto &I : BB) {
        AllocaInst *AI = dyn_cast<AllocaInst>(&I);
        if (AI) {
          Type *AT = AI->getAllocatedType();
          if (AT->isArrayTy() && (n = AT->getArrayNumElements()) <= 10) {
            Type *ET = AT->getArrayElementType();
            Store = M.getOrInsertFunction("$store", Type::getVoidTy(M.getContext()), ET, ET);
            Regs.clear();
            for (int i=0; i<n; i++) {
              Constant *Reg = M.getOrInsertGlobal("$reg"+to_string(i), ET);
              LoadInst *Inst = new LoadInst(ET, Reg, "load_reg"+to_string(i), F.getEntryBlock().getFirstNonPHI());
              Regs.push_back(Inst);
            }

            for (auto it = AI->user_begin(); it != AI->user_end(); it++) {
                auto *GEPI = dyn_cast<GetElementPtrInst>(*it);
                if (GEPI) {
                  changeUseOfGEPToSwitch(GEPI, F);

    
                } else {
                  assert("use not GEP!!");
                }
            }
          }
        }
      }
    }
  }

  return PreservedAnalyses::none();
}

    void Alloca2RegPass::changeUseOfGEPToSwitch(GetElementPtrInst *GEPI, Function &F) {
      for (auto it = GEPI->user_begin(); it != GEPI->user_end(); it++) {
        auto *I = *it;
        if (LoadInst* LI = dyn_cast<LoadInst>(I)) {
          // split current basic block in two, diving from LI
          auto *DownBB = LI->getParent();
          auto *UpBB = DownBB->splitBasicBlockBefore(LI, "splitted");
          // insert n simple basic blocks for switch
          vector<BasicBlock *> SwitchBBs;
          for (int i=0; i<n; i++) {
            auto *SwitchBB = BasicBlock::Create(F.getContext(), "splitted_bb"+to_string(i), &F, DownBB);
            IRBuilder<> builder(SwitchBB);
            auto *Br = BranchInst::Create(DownBB);
            builder.Insert(Br);
            SwitchBBs.push_back(SwitchBB);
          }
          // change terminator inst of upper splitted block to switch
          UpBB->getTerminator()->eraseFromParent();
          SwitchInst *SwI = SwitchInst::Create(GEPI->getOperand(2), SwitchBBs[0], n, UpBB);
          for (int i=0; i<n; i++) {
            ConstantInt *CI = ConstantInt::get(dyn_cast<IntegerType>(GEPI->getOperand(2)->getType()), i);
            SwI->addCase(CI, SwitchBBs[i]);
          }
          // insert phi node at the start of downward splitted block
          PHINode *Phi = PHINode::Create(LI->getType(), n,
                  "load.phi", DownBB->getFirstNonPHI());
          for (int i=0; i<n; i++) {
            Phi->addIncoming(Regs[i], SwitchBBs[i]);
          }
          LI->replaceAllUsesWith(Phi);
          // LI->eraseFromParent();
        } else if (StoreInst* SI = dyn_cast<StoreInst>(I)) {
          // split current basic block in two, diving from LI
          auto *DownBB = SI->getParent();
          auto *UpBB = DownBB->splitBasicBlockBefore(SI, "splitted");
          // insert n simple basic blocks for switch
          vector<BasicBlock *> SwitchBBs;
          for (int i=0; i<n; i++) {
            auto *SwitchBB = BasicBlock::Create(F.getContext(), "splitted_bb"+to_string(i), &F, DownBB);
            IRBuilder<> builder(SwitchBB);
            auto *Br = BranchInst::Create(DownBB);
            builder.Insert(Br);
            Value *Args[] = {Regs[i], SI->getOperand(0)};
            CallInst *StoreCI = CallInst::Create(Store, ArrayRef<Value *>(Args, 2), "", Br);
            SwitchBBs.push_back(SwitchBB);
          }
          // change terminator inst of upper splitted block to switch
          UpBB->getTerminator()->eraseFromParent();
          SwitchInst *SwI = SwitchInst::Create(GEPI->getOperand(2), SwitchBBs[0], n, UpBB);
          for (int i=0; i<n; i++) {
            ConstantInt *CI = ConstantInt::get(dyn_cast<IntegerType>(GEPI->getOperand(2)->getType()), i);
            SwI->addCase(CI, SwitchBBs[i]);
          }
        } else if (GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(I)) {
          changeUseOfGEPToSwitch(GI, F);
        }
      }
    }


} 