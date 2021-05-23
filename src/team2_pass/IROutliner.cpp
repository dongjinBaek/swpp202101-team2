#include "IROutliner.h"

/*******************************************
 * IROutlinerPass
 * Author: SeongHun Kim
********************************************/ 

using namespace std;
using namespace llvm;
using namespace team2_pass;

// consider outlining when the IR-reg count goes over this value
#define THRESHOLD_REGNO 30

namespace team2_pass {
// store <BasicBlock, reg count> pair
vector<pair<BasicBlock *, unsigned>> IROutlinerPass::blockRegCnt;
// visit history for DFS
vector<BasicBlock *> IROutlinerPass::visit;
// store all previous Values (potential parameters)
vector<Value *> IROutlinerPass::prevValues;

// fill the blockRegCnt vector by DFS toward successors
void IROutlinerPass::countRegs(BasicBlock *BB, unsigned predCnt){

    for(auto p : blockRegCnt)
        if(p.first == BB) return;   // loop; backtrack
    unsigned cnt = predCnt;
    // count instructions that are worth a reg
    for(auto &I : *BB){
        if (I.hasName()
            || (!I.hasName() && !I.getType()->isVoidTy()))
            cnt++;
    }
    blockRegCnt.push_back(make_pair(BB, cnt));
    for(auto *succ : successors(BB))
        countRegs(succ, cnt);
}

// recursively check whether I dominates BB and all blocks reachable by it
bool IROutlinerPass::domReachNoLoop(Instruction *I, BasicBlock *BB, DominatorTree &DT){

    if(find(visit.begin(), visit.end(), BB) != visit.end())
        return false;   // loop
    if(!DT.dominates(I, BB) && !(I->getParent()==BB))
        return false;   // not dominated
    visit.push_back(BB);
    Instruction *TI = dyn_cast<Instruction>(BB->getTerminator());
    // if one of the successors gives false, check can end immediately
    bool ret = true;
    for (unsigned s=0; ret && s < TI->getNumSuccessors(); ++s)
        ret = ret && domReachNoLoop(I, TI->getSuccessor(s), DT);
    return ret;
}

// fill the prevValues vector by DFS toward predecessors
void IROutlinerPass::gatherPrevValues(BasicBlock *BB){
    if(find(visit.begin(), visit.end(), BB) != visit.end())
        return;     // loop
    visit.push_back(BB);
    for(auto &I : *BB)
        if(!I.getType()->isVoidTy())
            prevValues.push_back(&I);
    for(auto *pred : predecessors(BB))
        gatherPrevValues(pred);
}

PreservedAnalyses IROutlinerPass::run(Module &M, ModuleAnalysisManager &MAM) {

    // this loop must be fixed when this code becomes able to create new function
    for (auto &F : M) {

        // outs() << "==" << F.getName() << '\n';

        // skip empty function
        if (F.empty() && !F.isMaterializable())
			continue;

        // build blockRegCnt
        blockRegCnt.clear();
        BasicBlock& EB = F.getEntryBlock();
        countRegs(&EB, 0);

        DominatorTree DT(F);
        BasicBlock *blockToSplit;
        Instruction *splitAt;
        
        for(auto p : blockRegCnt){
            // outs() << p.first->getName() << "\t\t" << p.second << '\n';
            if(p.second < THRESHOLD_REGNO){
                // outs() << "Not over threshold\n";
                continue;
            }
            // find a block that crosses the threshold count
            blockToSplit = p.first;
            bool split = false;
            visit.clear();
            // move to the instruction that exactly hits the threshold
            unsigned step = p.second - THRESHOLD_REGNO;
            BasicBlock::iterator it = blockToSplit->end();
            for(auto start=blockToSplit->begin();
                it!=start && step > 0; it--){
                    step--;
            }
            if(it == blockToSplit->end()) it--;
            // from that instruction, test each instruction with domReachNoLoop
            for(auto end=blockToSplit->end(); it!=end; it++){
                Instruction *I = &*it;
                if(I->getType()->isVoidTy()) continue;

                Value *V = dyn_cast<Value>(I);
                // outs() << "Check " << I->getName() << '\n';
                if(domReachNoLoop(I, blockToSplit, DT)){
                    // split here
                    splitAt = I;
                    // outs() << "Split point: ";
                    // outs() << I->getName() << '\n';
                    split = true;
                    break;
                }
            }
            // this block cannot be split
            if(!split) continue;

            // build prevValues from the split point
            visit.clear();
            visit.push_back(blockToSplit);
            prevValues.clear();
            for(auto &I : *blockToSplit){
                if(I.comesBefore(splitAt))
                    prevValues.push_back(dyn_cast<Value>(&I));
            }
            for(auto *pred : predecessors(blockToSplit))
                gatherPrevValues(pred);

            // for(auto *V : prevValues)
            //     outs() << V->getNameOrAsOperand() << '\n';

        }
    }
    return PreservedAnalyses::none();
}
}