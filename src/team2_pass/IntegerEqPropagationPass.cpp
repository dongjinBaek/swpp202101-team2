/*
    ArithmeticPass
*/

#include "IntegerEqPropagation.h"

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {

PreservedAnalyses IntegerEqPropagationPass::run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  //check the function again if update was made
  bool checkForUpdate = true;
  while (checkForUpdate) {
    checkForUpdate = false;
    for (auto &BB : F) {
      Value *X, *Y;
      BasicBlock *BBTrue, *BBFalse;
      ICmpInst::Predicate Pred;
      // match "br (icmp eq iN x, y) BB_true BB_false"
      if(match(BB.getTerminator(), m_Br(m_ICmp(Pred, m_Value(X), m_Value(Y)),
          m_BasicBlock(BBTrue), m_BasicBlock(BBFalse))) && Pred == ICmpInst::ICMP_EQ &&
          X->getType()->isIntegerTy() && Y->getType()->isIntegerTy() && X != Y && BBTrue != BBFalse)
      {
        BasicBlockEdge BBE(&BB, BBTrue);

        Constant *XC = dyn_cast<ConstantInt>(X), *YC = dyn_cast<ConstantInt>(Y);
        Argument *XA = dyn_cast<Argument>(X), *YA = dyn_cast<Argument>(Y);
        Instruction *XI = dyn_cast<Instruction>(X), *YI = dyn_cast<Instruction>(Y);
        if((XC || XA || XI) && (YC || YA || YI)) {
          Value *ChangeFrom, *ChangeTo;
          if (XC && YC) continue;
          if (XC ||
              (XA && YA && XA->getArgNo() < YA->getArgNo()) ||
              (XA && YI) ||
              (XI && YI && DT.dominates(XI, YI))) {
            ChangeFrom = Y;
            ChangeTo = X;
          }
          else {
            ChangeFrom = X;
            ChangeTo = Y;
          }
          if (changeUseIfEdgeDominates(ChangeFrom, ChangeTo, DT, BBE)) {
            checkForUpdate = true;
          }
        }
      }
    }
  }
  return PreservedAnalyses::all();
}

bool IntegerEqPropagationPass::changeUseIfEdgeDominates(Value *ChangeFrom, Value *ChangeTo,
                DominatorTree &DT, BasicBlockEdge &BBE) {
  bool changed = false;
  for (auto itr = ChangeFrom->use_begin(), end = ChangeFrom->use_end(); itr != end;) {
    Use &U = *itr++;
    if (DT.dominates(BBE, U)) {
      U.set(ChangeTo);
      changed = true;
    }
  }
  return changed;
}
}  // namespace backend