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
  for(auto &L : LoopInfo) {
    extractFromLoop(L);
  }

  return PreservedAnalyses::all();
}

 void ExtractFromLoopPass::extractFromLoop(Loop *L) {
auto v = L->getSubLoops();
if (v.size() > 0) {
  // outs() << "loop has sub: " << v.size() << "\n";
  for (auto sL : v) {
    extractFromLoop(sL);
  }
  return;
}


  BasicBlock *Preheader = L->getLoopPreheader();
    if(!Preheader) return;
    int cnt =0;
    outs() << "Pre: " << Preheader->getName() << "\n";
    for (auto &I : *Preheader) {
      outs() << I << "\n";
    }
    

    for(auto *BB : L->getBlocks()) {
      outs() << "Block: " << BB->getName() << "\n";
      for (auto &I : *BB) {
        outs() << "  " << I << "\n";
        auto LI = &I;
        if(auto LI = dyn_cast<LoadInst>(&I)){
          int updatedInLoop = 0;
          auto *loadedFrom = LI->getOperand(0);
          outs() << "-- It is Load!!\n";
           if(!(L->hasLoopInvariantOperands(LI))) {
             continue;
           }
             outs() << "--loop invariant\n";
          for (auto *BB2: L->getBlocks()) {
            for (auto &I2 : *BB2) {
              if(auto *SI = dyn_cast<StoreInst>(&I2)) {
                  if(SI->getOperand(1) == loadedFrom) {
                    updatedInLoop++;
                    outs() << "--updated at: " << I2 << "\n";
                  }
                }
              }
          }
          if (!updatedInLoop) {
            outs() << "  " << *LI << "\n";
            outs() << "--not updated in loop!\n";
            Instruction *InsertPtr;
            InsertPtr = Preheader->getTerminator();
            LI->moveBefore(InsertPtr);
            return;
          } else {
            auto *BBLatch = L->getLoopLatch();
            auto *BBHeader = L->getHeader();
            // not header && not latch
            if (BBLatch && BB != BBLatch && BB != BBHeader) {
              outs() << "header: " << *BBHeader << "\n";
              outs() << "latch: " << *BBLatch << "\n";
              if (updatedInLoop == 1) {
                for (auto &I2 : *BB) {
                  if(auto *SI = dyn_cast<StoreInst>(&I2)) {
                    if(SI->getOperand(1) == loadedFrom) {
                      PHINode *phi = PHINode::Create(SI->getOperand(0)->getType(), 2, "name" + to_string(cnt++), BBHeader->getFirstNonPHI());
                      outs() << *SI->getOperand(0) << "\n";
                      Instruction *InsertPtr;
                      InsertPtr = Preheader->getTerminator();
                      LI->moveBefore(InsertPtr);
                      // phi->addIncoming(ConstantInt::get(SI->getOperand(0)->getType(), 0), Preheader);
                      LI->replaceAllUsesWith(phi);
                      // phi->addIncoming(ConstantInt::get(SI->getOperand(0)->getType(), 0), BBLatch);
                      phi->addIncoming(LI, Preheader);
                      phi->addIncoming(SI->getOperand(0), BBLatch);
                      outs() << "phi: " << *phi << "\n";
                      // phi->insertBefore(BBHeader->getFirstNonPHI());
                        SmallVector<std::pair<BasicBlock *, BasicBlock *> > ExitEdges;
                      L->getExitEdges(ExitEdges);
                      for (auto Edge : ExitEdges) {
                        auto* BBExited = Edge.second;
                        SI->moveBefore(BBExited->getFirstNonPHI());
                        SI->setOperand(0, phi);
                      }
                      return;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
}

}  // namespace backend