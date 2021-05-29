#ifndef IR_OUTLINER_H
#define IR_OUTLINER_H

/*******************************************
 * IROutlinerPass
 * Author: SeongHun Kim
 * 
 * Find a suitable location to divide an IR function
 * so that the latter portion can be outlined into another function
 * 
 * Current strategy:
 * when the cumulative IR-reg count goes over some threshold
 * check if all reachable regions thereafter are dominated
 *      (and thus, have no loops or phis. This constraint is too strong!)
 * collect all values that appear before the split point
 *      TODO : Split the block into caller and callee
********************************************/ 

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include<vector>
#include<algorithm>

using namespace llvm;
using namespace std;

namespace team2_pass {
class IROutlinerPass : public PassInfoMixin<IROutlinerPass> {
// store <BasicBlock, reg count> pair
static vector<pair<BasicBlock *, unsigned>> blockRegCnt;
// visit history for DFS
static vector<BasicBlock *> visit;
// store all previous Values (potential parameters)
static vector<Value *> prevValues;
// fill the blockRegCnt vector by DFS toward successors
void countRegs(BasicBlock *, unsigned);
// check if a block meets the criteria for spliting
bool domReachNoLoop(Instruction *, BasicBlock *, DominatorTree &);
// fill the prevValues vector by DFS toward predecessors
void gatherPrevValues(BasicBlock *);
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};

}
#endif