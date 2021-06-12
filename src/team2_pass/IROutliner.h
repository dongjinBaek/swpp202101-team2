#ifndef IR_OUTLINER_H
#define IR_OUTLINER_H

/*******************************************
 * IROutlinerPass
 * Author: SeongHun Kim
 * 
 * Find a BasicBlock that would use too many registers
 * and outline its interior as a new function
 * 
 * Current strategy: Block-by-block, use-based
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
public:
    PreservedAnalyses run(Module &, ModuleAnalysisManager &);
};

}
#endif