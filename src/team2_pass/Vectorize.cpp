#include "Vectorize.h"

using namespace std;
using namespace llvm;
using namespace team2_pass;

#define DEBUG_TYPE "vectorize"

namespace team2_pass {

// get integer difference of two (pointer) value
Difference VectorizePass::getDifference(Value *V1, Value *V2) {
  if (V1 == V2)
    return {true, 0};

  // consider zext, sext as same value
  SExtInst *S1 = dyn_cast<SExtInst>(V1);
  SExtInst *S2 = dyn_cast<SExtInst>(V2);
  ZExtInst *Z1 = dyn_cast<ZExtInst>(V1);
  ZExtInst *Z2 = dyn_cast<ZExtInst>(V2);
  if (Z1) return getDifference(Z1->getOperand(0), V2);
  if (S1) return getDifference(S1->getOperand(0), V2);
  if (Z2) return getDifference(V1, Z2->getOperand(0));
  if (S2) return getDifference(V1, S2->getOperand(0));
  
  // constant
  ConstantInt *C1 = dyn_cast<ConstantInt>(V1);
  ConstantInt *C2 = dyn_cast<ConstantInt>(V2);
  if (C1 && C2) {
    int64_t i1 = C1->getSExtValue();
    int64_t i2 = C2->getSExtValue();
    return {true, i1 - i2};
  }
  
  // add, multiply
  BinaryOperator *BO1 = dyn_cast<BinaryOperator>(V1);
  BinaryOperator *BO2 = dyn_cast<BinaryOperator>(V2);
  if (BO1 && BO2 && BO1->getOpcode() == BO2->getOpcode()) {
    Difference diff1 = getDifference(BO1->getOperand(0), BO2->getOperand(0));
    Difference diff2 = getDifference(BO1->getOperand(1), BO2->getOperand(1));
    
    if (BO1->getOpcode() == Instruction::Add) {
      if (diff1.known && diff2.known)
        return {true, diff1.value + diff2.value};
    }
    else {
      if (diff1.known && diff2.known && diff1.value == 0 && diff2.value == 0)
        return {true, 0};
    }
  }

  // gep
  GetElementPtrInst *GEP1 = dyn_cast<GetElementPtrInst>(V1);
  GetElementPtrInst *GEP2 = dyn_cast<GetElementPtrInst>(V2);
  if (GEP1 && GEP2 && GEP1->getNumOperands() == 2 && GEP2->getNumOperands() == 2) {
    if (GEP1->getOperand(0) == GEP2->getOperand(0)) {
      return getDifference(GEP1->getOperand(1), GEP2->getOperand(1));
    }
  }

  // two instruction is different type
  for (int i = 0, sign = 1; i < 2; i++, sign *= -1) {
    // case 1: V1 = add (V2+diff) C
    if (BO1 && BO1->getOpcode() == Instruction::Add) {
      Value *OP1 = BO1->getOperand(0);
      ConstantInt *OP2 = dyn_cast<ConstantInt>(BO1->getOperand(1));
      Difference diff = getDifference(OP1, V2);
      if (OP2 && diff.known) 
        return {true, sign * (diff.value + OP2->getSExtValue())};
    }

    // case 2: V1 = gep V2 C
    if (GEP1 && GEP1->getNumOperands() == 2) {
      Value *OP1 = GEP1->getOperand(0);
      ConstantInt *OP2 = dyn_cast<ConstantInt>(GEP1->getOperand(1));
      if (OP2 && OP1 == V2) return {true, sign * OP2->getSExtValue()};
    }
    swap(V1, V2);
    swap(BO1, BO2);
    swap(GEP1, GEP2);
  }

  // unknown integer difference, may not share common base
  return {false, 0};
}

void VectorizePass::runOnBasicBlock(BasicBlock &BB) {
  Instruction *BaseI = nullptr;
  Instruction *NextBaseI = findNextBaseInstruction(BB.getFirstNonPHI());

  // loop until no more vectorize candidate
  // base instruction is always load or store
  while (NextBaseI) {
    BaseI = NextBaseI;
    NextBaseI = nullptr;
    bool isBaseLoad = isa<LoadInst>(BaseI);
    Value *BasePointer = isBaseLoad ? BaseI->getOperand(0) : BaseI->getOperand(1);
    Instruction *FirstUser = BaseI->user_empty() ? nullptr : BaseI->user_back();

    LLVM_DEBUG(dbgs() << "VCT: BaseI - " << *BaseI << '\n';
               if (FirstUser) dbgs() << "  First User - " << *FirstUser << '\n';);

    SmallVector<Instruction *, 8> VectInsts;
    SmallVector<int, 8> Offsets;
    Offsets.push_back(0);
    VectInsts.push_back(BaseI);

    // loop until vectorize condition is met, or end of basicblock
    Instruction *CurI = BaseI->getNextNonDebugInstruction();
    for (; CurI; CurI = CurI->getNextNonDebugInstruction()) {
      if (VectInsts.size() >= 8) {
        NextBaseI = findNextBaseInstruction(CurI);
        break;
      }

      LoadInst *LI = dyn_cast<LoadInst>(CurI);
      StoreInst *SI = dyn_cast<StoreInst>(CurI);
      CallInst *CI = dyn_cast<CallInst>(CurI);

      if (LI || SI) {
        Value *CurPointer = LI ? CurI->getOperand(0) : CurI->getOperand(1);
        Difference diff = getDifference(CurPointer, BasePointer);
        bool offsetExist = find(Offsets.begin(), Offsets.end(), diff.value) != Offsets.end();
        LLVM_DEBUG(dbgs() << "  Inst: " << *CurI << ", known: " << diff.known << ", value: " 
                   << diff.value << "  offsetExist: " << offsetExist << "\n";);

        // To sink vectorized load instruction, CurI should come before BaseI's first user instruction
        if (isBaseLoad && FirstUser &&
            FirstUser->getParent() == CurI->getParent() && 
            FirstUser->comesBefore(CurI)) {
          LLVM_DEBUG(dbgs() << "  CurI is after first user of BaseI" << "\n");
          NextBaseI = CurI;
          break;
        }

        // known-diff load/stores can be vectorized, except some case
        if (diff.known) {
          if (offsetExist) {
            NextBaseI = CurI;
            break;
          }

          if (isBaseLoad && LI || !isBaseLoad && SI) {
            VectInsts.push_back(CurI);
            Offsets.push_back(diff.value);
          }
        }
        // unknown-diff load/stores stops vectorize
        else {
          NextBaseI = CurI;
          break;
        }
      }
      // function call can access memory
      else if (CI) {
        Function *F = CI->getCalledFunction();
        string Fname = F->getName().str();
        bool writeOrRead = (Fname == "write" || Fname == "read");

        if (isBaseLoad && F->onlyReadsMemory() || writeOrRead)
          continue;
        else {
          LLVM_DEBUG(dbgs() << *CI << " calls memory accessing function\n";);
          NextBaseI = findNextBaseInstruction(CurI);
          break;
        }
      }
    }
    if (VectInsts.size() > 1) {
      Vectorize(VectInsts, Offsets, isBaseLoad);
    }
  }
}

// find next load/store instruction including I
Instruction *VectorizePass::findNextBaseInstruction(Instruction *I) {
  Instruction *NextBaseI = nullptr;
  for (; I; I = I->getNextNonDebugInstruction()) {
    if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
      NextBaseI = I;
      break;
    }
  }
  return NextBaseI;
}

void VectorizePass::Vectorize(SmallVector<Instruction *, 8> &VectInsts, SmallVector<int, 8> &Offsets, bool isLoad) {
  LLVM_DEBUG(
    dbgs() << "VCT: Instructions to vectorize - \n";
    for (int i = 0; i < VectInsts.size(); i++) {
      dbgs() << *VectInsts[i] << "\n";
    }
    for (int i = 0; i < Offsets.size(); i++) {
      dbgs() << Offsets[i] << " ";
    }
    dbgs() << "\n";
    dbgs() << "\n";
  );
  int vectorSize = VectInsts.size();
  if (!(vectorSize == 2 || vectorSize == 4 || vectorSize == 8)) {
    LLVM_DEBUG(dbgs() << "VCT: vectorSize is not 2, 4, or 8\n";);
    return;
  }

  for (int i = 0; i < vectorSize; i++)
    if (Offsets[i] != i) {
      LLVM_DEBUG(dbgs() << "VCT: offsets are not in order\n";);
      return;
    }

  // make call instruction(s) and replace uses if needed
  Instruction *InsertAfter = VectInsts.back();
  Value *Pointer = isLoad ? VectInsts[0]->getOperand(0) : VectInsts[0]->getOperand(1);
  if (isLoad) {
    Value *Args[] = {Pointer, ConstantInt::get(Int64Ty, (1 << vectorSize) - 1)};
    CallInst *CVLoad = CallInst::Create(VLoads[vectorSize], Args);
    CVLoad->insertAfter(InsertAfter);

    Instruction *LastInsert = CVLoad;
    for (int i = 0; i < vectorSize; i++) {
      Value *Args[] = {CVLoad, ConstantInt::get(Int64Ty, i)};
      CallInst *CExtract = CallInst::Create(ExtractElements[vectorSize], Args);
      CExtract->insertAfter(LastInsert);
      LastInsert = CExtract;

      VectInsts[i]->replaceAllUsesWith(CExtract);
    }
  }
  else {
    Value *Args[vectorSize + 2];
    for (int i = 0; i < vectorSize; i++) {
      Args[i] = VectInsts[i]->getOperand(0);
    }
    Args[vectorSize] = Pointer;
    Args[vectorSize + 1] = ConstantInt::get(Int64Ty, (1 << vectorSize) - 1);

    CallInst *CVStore = CallInst::Create(VStores[vectorSize], ArrayRef<Value *>(Args, vectorSize + 2));
    CVStore->insertAfter(InsertAfter);
  }
  
  for (int i = 0; i < vectorSize; i++)
      VectInsts[i]->eraseFromParent();

  LLVM_DEBUG(dbgs() << "VCT: vectorize done\n";);
}

void VectorizePass::declareFunctions(Module &M) {
  // declare base types
  VoidTy = Type::getVoidTy(M.getContext());
  Int64Ty = Type::getInt64Ty(M.getContext());
  Int64PtrTy = Type::getInt64PtrTy(M.getContext());
  VectIntTypes[2] = VectorType::get(Int64Ty, 2, false);
  VectIntTypes[4] = VectorType::get(Int64Ty, 4, false);
  VectIntTypes[8] = VectorType::get(Int64Ty, 8, false);

  // declare vload, vstore, extract_element functions
  VLoads[2] = M.getOrInsertFunction("vload2", VectIntTypes[2], Int64PtrTy, Int64Ty);
  VLoads[4] = M.getOrInsertFunction("vload4", VectIntTypes[4], Int64PtrTy, Int64Ty);
  VLoads[8] = M.getOrInsertFunction("vload8", VectIntTypes[8], Int64PtrTy, Int64Ty);
  ExtractElements[2] = M.getOrInsertFunction("extract_element2", Int64Ty, VectIntTypes[2], Int64Ty);
  ExtractElements[4] = M.getOrInsertFunction("extract_element4", Int64Ty, VectIntTypes[4], Int64Ty);
  ExtractElements[8] = M.getOrInsertFunction("extract_element8", Int64Ty, VectIntTypes[8], Int64Ty);
  VStores[2] = M.getOrInsertFunction("vstore2", VoidTy, Int64Ty, Int64Ty, Int64PtrTy, Int64Ty);
  VStores[4] = M.getOrInsertFunction("vstore4", VoidTy, Int64Ty, Int64Ty, Int64Ty, Int64Ty, Int64PtrTy, Int64Ty);
  VStores[8] = M.getOrInsertFunction("vstore8", VoidTy, Int64Ty, Int64Ty, Int64Ty, Int64Ty, Int64Ty, Int64Ty, Int64Ty, Int64Ty, Int64PtrTy, Int64Ty);
}

PreservedAnalyses VectorizePass::run(Module &M, ModuleAnalysisManager &MAM) {
  declareFunctions(M);

  for (Function &F : M) {
    for (BasicBlock &BB : F) {
      runOnBasicBlock(BB);
    }
  }
  return PreservedAnalyses::none();
}

} // namespace team2_pass

#undef DEBUG_TYPE