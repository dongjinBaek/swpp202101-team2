#include "IROutliner.h"

/*******************************************
 * IROutlinerPass
 * Author: SeongHun Kim
********************************************/ 

using namespace std;
using namespace llvm;
using namespace team2_pass;

// attempt outlining when the estimate score goes over this value
#define THR_CNT 60
// max number of arguments for outlined function
// slightly less than 16 for safety; argument selection may not be accurate
#define ARGMAX 13

namespace team2_pass {
PreservedAnalyses IROutlinerPass::run(Module &M, ModuleAnalysisManager &MAM) {

    // outline each function no more than once
    // or this pass might run too long
    vector<Function *> vecF;
    for (auto &F : M){
        vecF.push_back(&F);
    }

    for (auto &F : vecF) {
        //outs() << "==" << F->getName() << '\n';

        // skip empty function
        if (F->empty() && !F->isMaterializable())
			continue;

        // collect pairs <estimate reg count, BasicBlock *>
        vector<pair<unsigned, BasicBlock *>> blockRegCnt;
        blockRegCnt.clear();
        for (auto &BB : *F){
            unsigned cnt = 0;
            for(auto &I : BB){
                if (I.hasName()
                    || (!I.hasName() && !I.getType()->isVoidTy()))
                    cnt++;
            }
            blockRegCnt.push_back(make_pair(cnt, &BB));
        }
        // sort by cnt desc.
        sort(blockRegCnt);
        reverse(blockRegCnt.begin(), blockRegCnt.end());

        CodeExtractorAnalysisCache CEAC(*F);
        DominatorTree DT(*F);
        BasicBlock *BB, *newBB;
        Instruction *splitAt;
        unsigned regInstCnt;
        
        for(auto p : blockRegCnt){
            //outs() << p.first << " " << p.second->getName() << '\n';

            // only consider blocks going over threshold
            if((regInstCnt = p.first) < THR_CNT){
                break;
            }

            BB = p.second;
            set<Value *> newArgs;
            splitAt = nullptr;
            bool doOutline = false;

            // scan instructions backwards
            for(auto itr = --BB->end(),
                    start = BB->getFirstInsertionPt();
                    itr != start; itr--){
                Instruction *I = &*itr;
                //outs() << "I: " << I->getNameOrAsOperand() << "\n";
                if(!I || I->getType()->isVoidTy()){
                    //outs() << "Skip I\n";
                    continue;
                }

                // test split
                newBB = BB->splitBasicBlock(I);
                // I is now in the outline region
                newArgs.erase(I);

                for(auto &U : I->operands()){
                    if(!U) continue;
                    Value *V = U.get();
                    if(!V) continue;
                    //outs() << "V: " << V->getNameOrAsOperand() << "\n";

                    // args for original func must be passed as arg
                    if(isa<Argument>(V)){
                        //outs() << "Arg\n";
                        newArgs.insert(V);
                    }
                    // instructions out of outline region must be passed as arg
                    else if(isa<Instruction>(V)){
                        //outs() << "Inst\n";
                        Instruction *VI = dyn_cast<Instruction>(V);
                        if(!VI && VI->getParent() != newBB){
                            newArgs.insert(V);
                        }
                    }
                }
                // outs() << "New Args: " << newArgs.size() << '\n';
                // for(auto V : newArgs)
                //     outs() << V->getNameOrAsOperand() << " ";
                // outs() << '\n';

                // undo the test split
                MergeBlockIntoPredecessor(newBB);
                newBB = nullptr;

                // we now have enough arg
                if(newArgs.size() > ARGMAX){
                    doOutline = true;
                    break;
                }
                // update split point
                splitAt = I;
            }

            // perform function outlining
            if(doOutline && splitAt){
                newBB = BB->splitBasicBlock(splitAt);
                Function *newF = CodeExtractor(newBB).extractCodeRegion(CEAC);
                //outs() << "Split " << newF->getName() << " from " << F->getName() << '\n';
            }
        }
    }

    // Erase auto-generated calls to
    // @llvm.lifetime.start.p0i8 and @llvm.lifetime.end.p0i8
    for(auto &F : M){
        vector<Instruction *> toErase;
        for(auto &BB : F){
            for(auto &I: BB){
                CallInst *CI = dyn_cast<CallInst>(&I);
                if(!CI) continue;
                Function *CF = CI->getCalledFunction();
                if (CF->isIntrinsic() && !CF->getInstructionCount())
                    toErase.push_back(&I);
            }
        }
        for(auto &I : toErase)
            I->eraseFromParent();
    }
    return PreservedAnalyses::all();
}
}