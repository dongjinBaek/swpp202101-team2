/*
    ExtractFromLoopPass
*/

#include "ExtractFromLoopPass.h"

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {

PreservedAnalyses ExtractFromLoopPass::run(Function &F, FunctionAnalysisManager &FAM) {
 auto &LoopInfo = FAM.getResult<LoopAnalysis>(F);
 DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  for(auto &L : LoopInfo) {
    extractFromLoop(L, DT);
  }

  return PreservedAnalyses::none();
}

 void ExtractFromLoopPass::extractFromLoop(Loop *L, DominatorTree &DT) {
  auto v = L->getSubLoops();
  for (auto sL : v) {
    extractFromLoop(sL, DT);
  }

  BasicBlock *Preheader = L->getLoopPreheader();
    if(!Preheader) return;
    int cnt =0;

    for(auto *BB : L->getBlocks()) {
      for (auto &I : *BB) {
        if(auto LI = dyn_cast<LoadInst>(&I)){
          // handle only when address is loop invariant
          if(!(L->hasLoopInvariantOperands(LI))) {
             continue;
           }
          int loadCnt = 0, storeCnt = 0;
          // for store instruction with same address
          StoreInst *SI = NULL;
          auto *LoadedFrom = LI->getOperand(0);
          for (auto *BB2: L->getBlocks()) {
            for (auto &I2 : *BB2) {
              auto *TmpI = dyn_cast<StoreInst>(&I2);
              if (TmpI && TmpI->getOperand(1) == LoadedFrom) {
                storeCnt++;
                SI = TmpI;
              }
              auto *TmpI2 = dyn_cast<LoadInst>(&I2);
              if (TmpI2 && TmpI2->getOperand(0) == LoadedFrom) {
                loadCnt++;
              }
            }
          }
          // if no store instruction with same address in loop, extract load instruction
          if (storeCnt == 0) {
            LI->moveBefore(Preheader->getTerminator());
            return;
          } // if there is 1 load/store to loop invariant address, check for move
          else if (storeCnt == 1 && loadCnt == 1) {
            bool applyLoopExtract = true;
            if (!DT.dominates(LI, SI)) {
              applyLoopExtract = false;
            }
            if (LI->getType() != SI->getOperand(0)->getType()) {
              applyLoopExtract = false;
            }
            SmallVector<BasicBlock *> Latches;
            L->getLoopLatches(Latches);
            Instruction *InstTobeStored = dyn_cast<Instruction>(SI->getOperand(0));
            // check if inst tobe stored dominates all backedges
            for (auto *Latch : Latches) {
              if(!(DT.dominates(InstTobeStored, Latch) || InstTobeStored->getParent() == Latch)) {
                applyLoopExtract = false;
              }
            }
            if (applyLoopExtract) {
              auto *BBHeader = L->getHeader();
              PHINode *phi = PHINode::Create(LI->getType(), 1 + L->getNumBackEdges(),
                    "extract.loop" + to_string(cnt++), BBHeader->getFirstNonPHI());
              // move load instruction to the end of preheader block
              LI->moveBefore(Preheader->getTerminator());
              LI->replaceAllUsesWith(phi);
              // construct phinode
              phi->addIncoming(LI, Preheader);
              for (auto *Latch : Latches) {
                phi->addIncoming(SI->getOperand(0), Latch);
              }
              return;
            }
          }
        }
      }
    }
}

}  // namespace backend