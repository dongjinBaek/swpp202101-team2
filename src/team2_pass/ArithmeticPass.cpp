/*
    ArithmeticPass
*/

#include "ArithmeticPass.h"

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;

namespace backend {

PreservedAnalyses ArithmeticPass::run(Function &F, FunctionAnalysisManager &FAM) {

  propIntEq(F, FAM);
  Value *X;
  ConstantInt *C;
  vector<Instruction *> instsToRemove;
  vector<pair<Instruction *, Instruction *> > instPairsToChange;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (!match(&I, m_BinOp())) {
          continue;
        }
        if(match(&I, m_Shl(m_Value(X), m_ConstantInt(C))) && C->getZExtValue() < 64 ) {
          // shl(X, C) -> mul(X, 2^C)
          Value *op2 = ConstantInt::get(C->getType(),pow(2, C->getZExtValue()));
          Instruction *newInst = BinaryOperator::Create(Instruction::Mul, X, op2);
          instPairsToChange.push_back(make_pair(&I, newInst));
        } else if(match(&I, m_LShr(m_Value(X), m_ConstantInt(C))) && C->getZExtValue() < 64 ) {
          // lshr(X, C) -> udiv(X, 2^C)
          Value *op2 = ConstantInt::get(C->getType(),pow(2, C->getZExtValue()));
          Instruction *newInst = BinaryOperator::Create(Instruction::UDiv, X, op2);
          instPairsToChange.push_back(make_pair(&I, newInst));
        } else if(match(&I, m_Add(m_Value(X), m_Deferred(X))) ) {
          // add(X, X) -> mul(X, 2)
          Instruction *newInst = BinaryOperator::Create(Instruction::Mul, X, ConstantInt::get(X->getType(), 2));
          instPairsToChange.push_back(make_pair(&I, newInst));
        } else if((match(&I, m_Mul(m_ConstantInt(C),m_Value(X))) && C->isZero()) ||
           (match(&I, m_Mul(m_Value(X), m_ConstantInt(C))) && C->isZero()) ||
           (match(&I, m_And(m_ConstantInt(C),m_Value(X))) && C->isZero()) ||
           (match(&I, m_And(m_Value(X), m_ConstantInt(C))) && C->isZero()) ) {
          // mul(X, 0) -> 0
          // and(X, 0) -> 0
          Value *changeTo = ConstantInt::get(X->getType(),0);
          I.replaceAllUsesWith(changeTo);
          instsToRemove.push_back(&I);
        } else if(match(&I, m_Sub(m_Value(X), m_Deferred(X))) || 
            match(&I, m_Xor(m_Value(X), m_Deferred(X))) ) {
          // sub(X, X) -> 0
          Value *changeTo = ConstantInt::get(X->getType(),0);
          I.replaceAllUsesWith(changeTo);
          instsToRemove.push_back(&I);
        } else if(match(&I, m_UDiv(m_Value(X), m_Deferred(X))) || 
            match(&I, m_SDiv(m_Value(X), m_Deferred(X))) ) {
          // div(X, X) -> 1
          Value *changeTo = ConstantInt::get(X->getType(),1);
          I.replaceAllUsesWith(changeTo);
          instsToRemove.push_back(&I);
        } else if((match(&I, m_Add(m_ConstantInt(C),m_Value(X))) && C->isZero()) ||
           (match(&I, m_Mul(m_ConstantInt(C),m_Value(X))) && C->isOne()) ||
           (match(&I, m_Or(m_ConstantInt(C),m_Value(X))) && C->isZero()) ||
           (match(&I, m_Xor(m_ConstantInt(C),m_Value(X))) && C->isZero()) ||
           (match(&I, m_Add(m_Value(X), m_ConstantInt(C))) && C->isZero()) ||
           (match(&I, m_Sub(m_Value(X), m_ConstantInt(C))) && C->isZero()) ||
           (match(&I, m_Mul(m_Value(X), m_ConstantInt(C))) && C->isOne()) ||
           (match(&I, m_UDiv(m_Value(X), m_ConstantInt(C))) && C->isOne()) ||
           (match(&I, m_SDiv(m_Value(X), m_ConstantInt(C))) && C->isOne()) ||
           (match(&I, m_Or(m_Value(X), m_ConstantInt(C))) && C->isZero()) ||
           (match(&I, m_Xor(m_Value(X), m_ConstantInt(C))) && C->isZero())) {
          // add(X, 0) -> X
          // mul(X, 1) -> X
          // div(X, 1) -> X
          // or(X, 0) -> X
          // xor(X, 0) -> X
          I.replaceAllUsesWith(X);
          instsToRemove.push_back(&I);
        } else if(match(&I, m_And(m_Value(X), m_Deferred(X))) ||
           match(&I, m_Or(m_Value(X), m_Deferred(X))) ) {
          // and(X, X) -> X
          // or(X, X) -> X
          I.replaceAllUsesWith(X);
          instsToRemove.push_back(&I);
        }
      }
    }

  for (auto inst : instsToRemove){
    inst->eraseFromParent();
  }

  for (auto instPair : instPairsToChange){
    ReplaceInstWithInst(instPair.first, instPair.second);
  }

  return PreservedAnalyses::all();
}



void ArithmeticPass::propIntEq(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  //check the function again if update was made
  bool checkForUpdate = true;
  while (checkForUpdate) {
    checkForUpdate = false;
    for (auto &BB : F) {
      Value *X, *Y;
      ICmpInst::Predicate Pred;
      // match "br (icmp eq iN x, y) BB_true BB_false"
      if(match(BB.getTerminator(), m_Br(m_ICmp(Pred, m_Value(X), m_Value(Y)),
          m_BasicBlock(BB_true), m_BasicBlock(BB_false))) && Pred == ICmpInst::ICMP_EQ &&
          X->getType()->isIntegerTy() && Y->getType()->isIntegerTy() && X != Y && BB_true != BB_false)
      {
        BasicBlock *BBTrue = dyn_cast<BasicBlock>(TI->getOperand(2));
        BasicBlockEdge BBE(&BB, BBTrue);

        Constant *XC = dyn_cast<Constant>(X), *YC = dyn_cast<Constant>(Y);
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
          if (changeUseIfEdgeDominates(A, B, DT, BBE)) {
            checkForUpdate = true;
          }
        }
      }
    }
  }
}

bool ArithmeticPass::changeUseIfEdgeDominates(Value *ChangeFrom, Value *ChangeTo, DominatorTree &DT, BasicBlockEdge &BBE) {
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