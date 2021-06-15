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

  // consider zext, sext, inttoptr, bitcast, trunc as same value
  for (int i = 0, sign = 1; i < 2; i++, sign *= -1) {
    if (isa<SExtInst>(V1) || isa<ZExtInst>(V1) || isa<IntToPtrInst>(V1) || isa<BitCastInst>(V1) || isa<TruncInst>(V1)) {
      Instruction *I = dyn_cast<Instruction>(V1);
      Difference diff = getDifference(I->getOperand(0), V2);
      return {diff.known, sign * diff.value};
    }
    swap (V1, V2);
  }

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
  if (GEP1 && GEP2) {
    if (GEP1->getNumOperands() == 2 && GEP2->getNumOperands() == 2 &&
        GEP1->getOperand(0) == GEP2->getOperand(0)) {
      return getDifference(GEP1->getOperand(1), GEP2->getOperand(1));
    }
    else if (GEP1->getNumOperands() == 3 && GEP2->getNumOperands() == 3 &&
             GEP1->getOperand(0) == GEP2->getOperand(0) &&
             GEP1->getOperand(1) == GEP2->getOperand(1)) {
      return getDifference(GEP1->getOperand(2), GEP2->getOperand(2));
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

void VectorizePass::runOnBasicBlock(BasicBlock &BB, bool sinkLoadUsers) {
  if (sinkLoadUsers)
    sinkAllLoadUsers(BB);

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
    SmallVector<int, 8> VOffsets;     // offsets to vectorize
    SmallVector<int> AOffsets;        // all offsets appeared
    VOffsets.push_back(0);
    AOffsets.push_back(0);
    VectInsts.push_back(BaseI);

    // loop until vectorize condition is met, or end of basicblock
    Instruction *CurI = BaseI->getNextNonDebugInstruction();
    for (; CurI; CurI = CurI->getNextNonDebugInstruction()) {
      if (VectInsts.size() >= 8) {
        NextBaseI = NextBaseI ? NextBaseI : findNextBaseInstruction(CurI);
        break;
      }

      LoadInst *LI = dyn_cast<LoadInst>(CurI);
      StoreInst *SI = dyn_cast<StoreInst>(CurI);
      CallInst *CI = dyn_cast<CallInst>(CurI);

      if (LI || SI) {
        Value *CurPointer = LI ? CurI->getOperand(0) : CurI->getOperand(1);
        if (CurPointer->getType() != Int64PtrTy) continue;
        Difference diff = getDifference(CurPointer, BasePointer);
        bool offsetExist = find(AOffsets.begin(), AOffsets.end(), diff.value) != AOffsets.end();
        LLVM_DEBUG(dbgs() << "  Inst: " << *CurI << ", known: " << diff.known << ", value: " 
                   << diff.value << "  offsetExist: " << offsetExist << "\n";);

        // To sink vectorized load instruction, CurI should come before BaseI's first user instruction
        if (isBaseLoad && FirstUser &&
            FirstUser->getParent() == CurI->getParent() && 
            FirstUser->comesBefore(CurI)) {
          LLVM_DEBUG(dbgs() << "  CurI is after first user of BaseI" << "\n");
          NextBaseI = NextBaseI ? NextBaseI : CurI;
          break;
        }

        // known-diff load/stores can be vectorized, except some case
        if (diff.known) {
          if (offsetExist) {
            NextBaseI = NextBaseI ? NextBaseI : CurI;
            break;
          }

          AOffsets.push_back(diff.value);
          if (isBaseLoad && LI || !isBaseLoad && SI) {
            VectInsts.push_back(CurI);
            VOffsets.push_back(diff.value);
          }
          else {
            NextBaseI = NextBaseI ? NextBaseI : CurI;
          }
        }
        // unknown-diff load/stores stops vectorize
        else {
          NextBaseI = NextBaseI ? NextBaseI : CurI;
          break;
        }
      }
      // function call can access memory
      else if (CI) {
        Function *F = CI->getCalledFunction();
        StringRef name = F->getName();
        if (isBaseLoad && F->onlyReadsMemory() ||
            (!doesAccessMemory(CI) || name == "read" || name == "write"))
          continue;
        else {
          LLVM_DEBUG(dbgs() << *CI << " calls memory accessing function\n";);
          NextBaseI = NextBaseI ? NextBaseI : findNextBaseInstruction(CurI);
          break;
        }
      }
    }
    if (VectInsts.size() > 1) {
      Vectorize(VectInsts, VOffsets, isBaseLoad);
    }
  }
}

// find next load/store instruction including I
Instruction *VectorizePass::findNextBaseInstruction(Instruction *I) {
  Instruction *NextBaseI = nullptr;
  for (; I; I = I->getNextNonDebugInstruction()) {
    LoadInst *LI = dyn_cast<LoadInst>(I);
    StoreInst *SI = dyn_cast<StoreInst>(I);
    if (LI || SI) {
      Type *PointerType = LI ? LI->getPointerOperandType() : SI->getPointerOperandType();
      if (PointerType == Int64PtrTy) {
        NextBaseI = I;
        break;
      }
    }
  }
  return NextBaseI;
}

void VectorizePass::Vectorize(SmallVector<Instruction *, 8> &VectInsts, SmallVector<int, 8> &Offsets, bool isLoad) {
  int vectorSize = VectInsts.size();

  assert (vectorSize == Offsets.size() && "Offset size should match VectInst size");
  assert (vectorSize >= 2 && vectorSize <= 8 && "VectInst size range is 2 <= size <= 8");
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

  // sort in offset order, adjust offset
  Instruction *InsertAfter = VectInsts.back();
  SmallVector<pair<int, Instruction *>, 8> OffsetInstPairs;
  for (int i = 0; i < vectorSize; i++) {
    OffsetInstPairs.push_back(make_pair(Offsets[i], VectInsts[i]));
  }
  sort(OffsetInstPairs);
  int baseOffset = OffsetInstPairs[0].first;
  for (int i = 0; i < vectorSize; i++) {
    VectInsts[i] = OffsetInstPairs[i].second;
    Offsets[i] = OffsetInstPairs[i].first - baseOffset;
  }

  // build mask
  int offsetRange = Offsets[vectorSize-1] - Offsets[0];
  int targetSize, mask;
  int i_to_idx[8], mask_arr[8] = {1, 1, 1, 1, 1, 1, 1, 1};
  if (offsetRange >= 8) {
    LLVM_DEBUG(dbgs() << "Offsets should fit in 8 bytes to vectorize\n");
    return;
  }
  if (offsetRange == 1) targetSize = 2;
  else if (offsetRange < 4) targetSize = 4;
  else targetSize = 8;
  mask = (1 << targetSize) - 1;
  for (int i = 0, idx = 0; i < targetSize; i++) {
    if (idx == vectorSize || Offsets[idx] != i) {
      mask ^= 1 << i;
      mask_arr[i] = 0;
    }
    else {
      i_to_idx[i] = idx;
      idx++;
    }
  }

  // make call instruction(s) and replace uses if needed
  Value *Pointer = isLoad ? VectInsts[0]->getOperand(0) : VectInsts[0]->getOperand(1);
  if (isLoad) {
    Value *Args[] = {Pointer, ConstantInt::get(Int64Ty, mask)};
    CallInst *CVLoad = CallInst::Create(VLoads[targetSize], Args);
    CVLoad->insertAfter(InsertAfter);

    Instruction *LastInsert = CVLoad;
    for (int i = 0; i < targetSize; i++) {
      if (mask_arr[i]) {
        Value *Args[] = {CVLoad, ConstantInt::get(Int64Ty, i)};
        CallInst *CExtract = CallInst::Create(ExtractElements[targetSize], Args);
        CExtract->insertAfter(LastInsert);
        LastInsert = CExtract;

        VectInsts[i_to_idx[i]]->replaceAllUsesWith(CExtract);
        loadVectorized = true;
      }
    }
  }
  else {
    Value *Args[targetSize + 2];
    for (int i = 0; i < targetSize; i++) {
      if (mask_arr[i])
        Args[i] = VectInsts[i_to_idx[i]]->getOperand(0);
      else
        Args[i] = ConstantInt::get(Int64Ty, 0);
    }
    Args[targetSize] = Pointer;
    Args[targetSize + 1] = ConstantInt::get(Int64Ty, mask);
    CallInst *CVStore = CallInst::Create(VStores[targetSize], ArrayRef<Value *>(Args, targetSize + 2));

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

void VectorizePass::sinkAllLoadUsers(BasicBlock &BB) {
  for (auto it = BB.begin(); it != BB.end(); it++) {
    LoadInst *LI = dyn_cast<LoadInst>(&(*it));
    if (!LI) continue;

    vector<Instruction *> users;
    for(auto it = LI->user_begin(); it != LI->user_end(); it++) {
      Instruction *UserI = dyn_cast<Instruction>(*it);
      if (UserI->getParent() == &BB && LI->comesBefore(UserI))
        users.push_back(UserI);
    }

    for(auto it = users.begin(); it != users.end(); it++)
      sinkRecursive(BB, *it);
  }
}

// sink given instruction recursively in post-order
void VectorizePass::sinkRecursive(BasicBlock &BB, Instruction *I) {
  static set<Instruction *> sinked;
  if (sinked.find(I) != sinked.end()) return;
  else sinked.insert(I);

  LLVM_DEBUG(dbgs() << "sinkRecursive: " << BB.getName() << " " << *I << "\n");
  Instruction *MaxSinkI = BB.getTerminator();

  // sink all children recursively
  for (auto it = I->user_begin(); it != I->user_end(); it++) {
    Instruction *UserI = dyn_cast<Instruction>(*it);

    if (UserI->getParent() == &BB && I->comesBefore(UserI)) {
      sinkRecursive(BB, UserI);
      MaxSinkI = MaxSinkI->comesBefore(UserI) ? MaxSinkI : UserI;
    }
  }

  // find where to sink current instruction if access memory
  LoadInst *LI = dyn_cast<LoadInst>(I);
  StoreInst *SI = dyn_cast<StoreInst>(I);
  CallInst *CI = dyn_cast<CallInst>(I);

  if (LI || SI || (CI && doesAccessMemory(CI))) {
    // find next user
    Instruction *NextUI = nullptr;
    for (auto it = I->user_begin(); it != I->user_end(); it++) {
      User *user = *it;
      if (Instruction *UserI = dyn_cast<Instruction>(user))
        if (UserI->getParent() == &BB && !isa<PHINode>(UserI))
          NextUI = NextUI ? (NextUI->comesBefore(UserI) ? NextUI : UserI) : UserI;
    }
    if (NextUI && NextUI->getParent() == &BB)
      MaxSinkI = MaxSinkI->comesBefore(NextUI) ? MaxSinkI : NextUI;

    // find next unknown-difference or equal pointer memory access
    Instruction *NextMI = findNextMemoryInstruction(I->getNextNonDebugInstruction());
    while (NextMI) {
      LoadInst *NLI = dyn_cast<LoadInst>(NextMI);
      StoreInst *NSI = dyn_cast<StoreInst>(NextMI);
      CallInst *NCI = dyn_cast<CallInst>(NextMI);

      if (CI || NCI) break;   // memory accessing call -> unknown-difference

      Value *CurP = LI ? I->getOperand(0) : I->getOperand(1);
      Value *NextP = NLI ? NextMI->getOperand(0) : NextMI->getOperand(1);
      Difference d = getDifference(CurP, NextP);

      if (d.known && d.value == 0) break;
      else if (!d.known) break;
      else NextMI = findNextMemoryInstruction(NextMI->getNextNonDebugInstruction());
    }
    if (NextMI)
      MaxSinkI = MaxSinkI->comesBefore(NextMI) ? MaxSinkI : NextMI;
  }

  // sink current instruction
  I->moveBefore(MaxSinkI);
}

// find next load/store/call(except 'read' and 'write') instruction including I
Instruction *VectorizePass::findNextMemoryInstruction(Instruction *I) {
  Instruction *NextI = nullptr;
  for (; I; I = I->getNextNonDebugInstruction()) {
    LoadInst *LI = dyn_cast<LoadInst>(I);
    StoreInst *SI = dyn_cast<StoreInst>(I);
    CallInst *CI = dyn_cast<CallInst>(I);
    if (LI || SI) {
      NextI = I;
      break;
    }
    if (CI) {
      Function *F = CI->getCalledFunction();
      if (doesAccessMemory(CI)) {
        NextI = I;
        break;
      }
    }
  }
  return NextI;
}

bool VectorizePass::doesAccessMemory(CallInst *CI) {
  Function *F = CI->getCalledFunction();
  return !F->doesNotAccessMemory();
}

void VectorizePass::runOnModule(Module &M, bool sinkLoadUsers) {
  declareFunctions(M);
  for (Function &F : M) {
    for (BasicBlock &BB : F) {
      runOnBasicBlock(BB, sinkLoadUsers);
    }
  }
}

PreservedAnalyses VectorizePass::run(Module &M, ModuleAnalysisManager &MAM) {
  // test for cloned module
  runOnModule(*CloneModule(M), true);
  // if load was not vetorized, don't sink
  runOnModule(M, loadVectorized);
  return PreservedAnalyses::none();
}

} // namespace team2_pass

#undef DEBUG_TYPE
