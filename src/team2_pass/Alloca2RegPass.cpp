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
            InstsToRemove = vector<Instruction *>();
            cnt =0;
  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        AllocaInst *AI = dyn_cast<AllocaInst>(&I);
        if (AI) {
          if (canChangeAlloca2Reg(AI)) {
            Type *ET = AI->getAllocatedType()->getArrayElementType();
            elementBitWidth = ET->getIntegerBitWidth();
            FunctionCallee S = M.getOrInsertFunction("$store", Type::getVoidTy(M.getContext()),
                                                                   ET, ET, Type::getInt32Ty(M.getContext()));
            Regs.clear();
            InstsToRemove.clear();
            for (int i=0; i<n; i++) {
              Constant *Reg = M.getOrInsertGlobal("$reg"+to_string(cnt++), ET);
              LoadInst *Inst = new LoadInst(ET, Reg, "load_reg"+to_string(i), F.getEntryBlock().getFirstNonPHI());
              Regs.push_back(Inst);
            }

            for (auto it = AI->user_begin(); it != AI->user_end(); it++) {
                auto *GEPI = dyn_cast<GetElementPtrInst>(*it);
                if (GEPI) {
                  changeUseOfGEPToSwitch(GEPI, F, S);
                } else {
                  assert("use not GEP!!");
                }
            }
            for (auto *I : InstsToRemove) {
              I->eraseFromParent();
            }
          }
        }
      }
    }
  }

  return PreservedAnalyses::none();
}

void Alloca2RegPass::changeUseOfGEPToSwitch(GetElementPtrInst *GEPI, Function &F, FunctionCallee &S) {
  for (auto it = GEPI->user_begin(); it != GEPI->user_end(); it++) {
    auto *I = *it;
    if (LoadInst* LI = dyn_cast<LoadInst>(I)) {
      // split current basic block in two, diving from LI
      auto *DownBB = LI->getParent();
      auto *UpBB = DownBB->splitBasicBlockBefore(LI, "splitted" + to_string(cnt++));
      // insert n simple basic blocks for switch
      vector<BasicBlock *> SwitchBBs;
      for (int i=0; i<n; i++) {
        auto *SwitchBB = BasicBlock::Create(F.getContext(), "splitted_bb"+to_string(cnt++), &F, DownBB);
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
      InstsToRemove.push_back(dyn_cast<Instruction>(I));
    }
    else if (StoreInst* SI = dyn_cast<StoreInst>(I)) {
      // split current basic block in two, diving from LI
      auto *DownBB = SI->getParent();
      auto *UpBB = DownBB->splitBasicBlockBefore(SI, "splitted"+ to_string(cnt++));
      // insert n simple basic blocks for switch
      vector<BasicBlock *> SwitchBBs;
      for (int i=0; i<n; i++) {
        auto *SwitchBB = BasicBlock::Create(F.getContext(), "splitted_bb"+ to_string(cnt++), &F, DownBB);
        IRBuilder<> builder(SwitchBB);
        auto *Br = BranchInst::Create(DownBB);
        builder.Insert(Br);
        // insert $store(reg, val, bitwidth) instead of SI, and handle it in backend
        Value *Args[] = {Regs[i], SI->getOperand(0), 
                          ConstantInt::get(S.getFunctionType()->getParamType(2), elementBitWidth)};
        CallInst *StoreCI = CallInst::Create(S, ArrayRef<Value *>(Args, 3), "", Br);
        SwitchBBs.push_back(SwitchBB);
      }
      // change terminator inst of upper splitted block to switch
      UpBB->getTerminator()->eraseFromParent();
      SwitchInst *SwI = SwitchInst::Create(GEPI->getOperand(2), SwitchBBs[0], n, UpBB);
      for (int i=0; i<n; i++) {
        ConstantInt *CI = ConstantInt::get(dyn_cast<IntegerType>(GEPI->getOperand(2)->getType()), i);
        SwI->addCase(CI, SwitchBBs[i]);
      }
      InstsToRemove.push_back(dyn_cast<Instruction>(I));
    }
    else if (GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(I)) {
      changeUseOfGEPToSwitch(GI, F, S);
    }
  }
}

bool Alloca2RegPass::canChangeAlloca2Reg(AllocaInst *AI) {
  Type *AT = AI->getAllocatedType();
  return AT->isArrayTy() && (n = AT->getArrayNumElements()) <= 10 && AT->getArrayElementType()->isIntegerTy();
}


} 