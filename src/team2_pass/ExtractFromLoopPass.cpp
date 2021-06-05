/*
    ExtractFromLoopPass - extract loop invariant load instructions in some cases.
    1. there is no store instruction to same address in loop (LICM also does this, but not to global variables)
    2. there is 1 store instruction to same address in loop, and all backedges are dominated by store,
    extract load instruction and make a phi node at start of the loop.
*/

#include "ExtractFromLoopPass.h"
#include "Vectorize.h"

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace team2_pass {

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
            if (TmpI) {
              Difference difference = VectorizePass::getDifference(TmpI->getOperand(1), LoadedFrom);
              if (difference.known) {
                if (difference.value == 0) {
                  storeCnt++;
                  SI = TmpI;
                }
              }
              else
                return;
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
            SmallVector<std::pair<BasicBlock *, BasicBlock *> > ExitEdges;
            L->getExitEdges(ExitEdges);
            if (ExitEdges.size() == 1) {
              BasicBlock *Exiting = ExitEdges[0].first;
              BasicBlock *Exited = ExitEdges[0].second;
              if (DT.dominates(Preheader, Exited)) {
                SI->moveBefore(Exited->getFirstNonPHI());
                PHINode *phi2 = PHINode::Create(SI->getOperand(0)->getType(), 1,
                  "store.phi" + to_string(cnt++), Exited->getFirstNonPHI());
                phi2->addIncoming(phi, Exiting);
                SI->setOperand(0, phi2);
                SI->setOperand(1, LI->getOperand(0));
              }
            }
            return;
          }
        }
      }
    }
  }
}

}  // namespace backend