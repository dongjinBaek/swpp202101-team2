/*
    GEPUnpackPass
*/

#include "GEPUnpack.h"

using namespace llvm;
using namespace std;

namespace backend {

// Return sizeof(T) in bytes.
static
unsigned getAccessSize(Type *T, unsigned int maxBW) {
  if (isa<PointerType>(T))
    return 8;
  else if (isa<IntegerType>(T)) {
    unsigned int currBW = T->getIntegerBitWidth() == 1 ? 1 : (T->getIntegerBitWidth() / 8);
    return 8 * currBW / maxBW;
  }
  else if (isa<ArrayType>(T))
    return getAccessSize(T->getArrayElementType(), maxBW) * T->getArrayNumElements();
  assert(false && "Unsupported access size type!");
}

static unsigned int getMaxBitWidth(Instruction &I) {
    unsigned int maxBW = 1;
    for (auto &U : I.uses()) {
    User *user = U.getUser();
    if (LoadInst *LI = dyn_cast<LoadInst>(user)) {
      Type *T = LI->getType();
      unsigned int currBW = isa<IntegerType>(T) ?
          (T->getIntegerBitWidth() == 1 ? 1 : (T->getIntegerBitWidth() / 8)) : 8;
      maxBW = max(maxBW, currBW);
    }
    else if (StoreInst *SI = dyn_cast<StoreInst>(user)) {
      Type *T = SI->getOperand(0)->getType();
      unsigned int currBW = isa<IntegerType>(T) ?
          (T->getIntegerBitWidth() == 1 ? 1 : (T->getIntegerBitWidth() / 8)) : 8;
      maxBW = max(maxBW, currBW);
    }
    else if (BitCastInst *BCI = dyn_cast<BitCastInst>(user)) {
      unsigned int currBW = getMaxBitWidth(*BCI);
      maxBW = max(maxBW, currBW);
    }
    else {
      Type *T = I.getType()->getPointerElementType();
      unsigned int currBW = isa<IntegerType>(T) ?
          (T->getIntegerBitWidth() == 1 ? 1 : (T->getIntegerBitWidth() / 8)) : 8;
      maxBW = max(maxBW, currBW);
    }
  }
  return maxBW;
}

static void collectLoadStoreInst(Instruction *I, vector<LoadInst *> &LoadList,
  vector<StoreInst *> &StoreList, vector<pair<Instruction *, Instruction *> > &OtherList,
                                            vector<Instruction *> &EraseList) {
  for (auto &U : I->uses()) {
    User *user = U.getUser();
    if (LoadInst *LI = dyn_cast<LoadInst>(user)) {
      LoadList.push_back(LI);
      EraseList.push_back(LI);
    }
    else if (StoreInst *SI = dyn_cast<StoreInst>(user)) {
      StoreList.push_back(SI);
      EraseList.push_back(SI);
    }
    else if (BitCastInst *BCI = dyn_cast<BitCastInst>(user)) {
      collectLoadStoreInst(BCI, LoadList, StoreList, OtherList, EraseList);
      EraseList.push_back(BCI);
    }
    else if (Instruction *userI = dyn_cast<Instruction>(user))
      OtherList.push_back(make_pair(userI, I));
  }
}

PreservedAnalyses GEPUnpackPass::run(Module &M, ModuleAnalysisManager &MAM) {
  LLVMContext &Context = M.getContext();
  IntegerType *Int64Ty = Type::getInt64Ty(Context);
  map<pair<Type *, Type *>, string> LoadMap, StoreMap;
  unsigned int LoadNum = 0, StoreNum = 0;

  Constant *temp = M.getOrInsertGlobal("$temp", Int64Ty);

  vector<Function *> Func;
  for (Function &F : M)
    Func.push_back(&F);

  // Unpack GEPs.
  for (Function *F : Func) {
    vector<Instruction *> EraseList;
    for (auto it = inst_begin(F); it != inst_end(F); ++it) {
      Instruction &I = *it;
      if (I.getOpcode() != Instruction::GetElementPtr) continue;
      unsigned int maxBW = getMaxBitWidth(I);
      GetElementPtrInst *GI = dyn_cast<GetElementPtrInst>(&I);
      Value *ptrOp = GI->getPointerOperand();
      Type *curr = ptrOp->getType();
      curr = curr->getPointerElementType();

      Value *var = NULL;
      uint64_t cst = 0UL;
      ConstantInt *CI;
      BinaryOperator *BO;
      for (auto opIt = GI->idx_begin(); opIt != GI->idx_end(); ++opIt) {
        Value *op = *opIt;
        unsigned size = getAccessSize(curr, maxBW);
        if ((CI = dyn_cast<ConstantInt>(op)))
          cst += CI->getZExtValue() * size;
        else if ((BO = dyn_cast<BinaryOperator>(op)) && BO->getOpcode() ==
            Instruction::Add && (CI = dyn_cast<ConstantInt>(BO->getOperand(1)))) {
          Instruction *mul = BinaryOperator::CreateMul(BO->getOperand(0),
              ConstantInt::get(Int64Ty, size, true));
          mul->insertBefore(&I);
          if (var) {
            Instruction *add = BinaryOperator::CreateAdd(var, mul);
            add->insertBefore(&I);
            var = add;
          }
          else var = mul;
          cst += CI->getZExtValue() * size;
        }
        else {
          Instruction *mul = BinaryOperator::CreateMul(
              op, ConstantInt::get(Int64Ty, size, true));
          mul->insertBefore(&I);
          if (var) {
            Instruction *add = BinaryOperator::CreateAdd(var, mul);
            add->insertBefore(&I);
            var = add;
          }
          else var = mul;
        }
        if (curr->isArrayTy()) curr = curr->getArrayElementType();
      }
      if (!var) var = ConstantInt::get(Int64Ty, 0, true);
      Value *cstV = ConstantInt::get(Int64Ty, cst, true);
      vector<LoadInst *> LoadList;
      vector<StoreInst *> StoreList;
      vector<pair<Instruction *, Instruction *> > OtherList;
      collectLoadStoreInst(GI, LoadList, StoreList, OtherList, EraseList);
      
      Type *srcTy = ptrOp->getType();
      for (LoadInst *LI : LoadList) {
        Type *destTy = LI->getType();
        auto pair = make_pair(srcTy, destTy);
        if (LoadMap.find(pair) == LoadMap.end())
          LoadMap[pair] = "$load." + to_string(LoadNum++);
        FunctionCallee Load = M.getOrInsertFunction(LoadMap[pair],
                                destTy, srcTy, Int64Ty, Int64Ty, Int64Ty);
        IRBuilder<> IRB(LI);
        LoadInst *tempLI = IRB.CreateLoad(temp);
        Value *Args[] = {ptrOp, var, cstV, tempLI};
        CallInst *Call = CallInst::Create(Load, ArrayRef<Value *>(Args, 4));
        Call->insertBefore(LI);
        LI->replaceAllUsesWith(Call);
      }
      for (StoreInst *SI : StoreList) {
        Type *destTy = SI->getOperand(0)->getType();
        auto pair = make_pair(srcTy, destTy);
        if (StoreMap.find(pair) == StoreMap.end())
          StoreMap[pair] = "$store." + to_string(StoreNum++);
        FunctionCallee Store = M.getOrInsertFunction(StoreMap[pair],
            Type::getVoidTy(Context), destTy, srcTy, Int64Ty, Int64Ty, Int64Ty);
        IRBuilder<> IRB(SI);
        LoadInst *tempLI = IRB.CreateLoad(temp);
        Value *Args[] = {SI->getOperand(0), ptrOp, var, cstV, tempLI};
        CallInst *Call = CallInst::Create(Store, ArrayRef<Value *>(Args, 5));
        Call->insertBefore(SI);
      }
      if (!OtherList.empty()) {
          Instruction *pti = CastInst::CreateBitOrPointerCast(ptrOp, Int64Ty);
          pti->insertBefore(&I);
          if (!isa<ConstantInt>(var)) {
            pti = BinaryOperator::CreateAdd(pti, var);
            pti->insertBefore(&I);
          }
          if (cst) {
            pti = BinaryOperator::CreateAdd(pti, cstV);
            pti->insertBefore(&I);
          }
          map<Type *, Instruction *> TypeMap;
          for (auto &pair : OtherList) {
            Instruction *user = pair.first;
            Instruction *from = pair.second;
            if (TypeMap.find(from->getType()) == TypeMap.end()) {
              Instruction *to = CastInst::CreateBitOrPointerCast(pti, from->getType());
              TypeMap[from->getType()] = to;
              to->insertBefore(&I);
            }
            user->replaceUsesOfWith(from, TypeMap[from->getType()]);
          }
      }
      EraseList.push_back(&I);
    }
    for (Instruction *I : EraseList)
      I->eraseFromParent();
  }

  return PreservedAnalyses::none();
}
}  // namespace backend
