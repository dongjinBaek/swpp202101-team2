#include "ScaleToInt64.h"

using namespace std;
using namespace llvm;
using namespace team2_pass;

#define DEBUG_TYPE "scaletoi64"

namespace team2_pass {

static Type *Int32Ty, *Int64Ty, *Int32PtrTy, *Int64PtrTy;

// get minimum bitwidth of all loads/stores
static int getMinBitwidth(Module &M) {
    int bitwidth = 64;
    for (Function &F : M) {
    for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
        LoadInst *LI = dyn_cast<LoadInst>(&I);
        StoreInst *SI = dyn_cast<StoreInst>(&I);

        int b;
        Type *T = nullptr;
        if (LI) T = LI->getType();
        else if (SI) T = SI->getOperand(0)->getType();
        if (!T || T->isPointerTy())
            continue;
        b = T->getIntegerBitWidth();
        bitwidth = bitwidth < b ? bitwidth : b;
    }}}
    return bitwidth;
}

// // replace all in loads/stores to i64 loads/stores
// static void replaceLoadStores(Module &M) {
//     // collect instructions to replace, because iterator breaks when replacing
//     vector<Instruction *> instructions;
//     for (Function &F : M) {
//     for (BasicBlock &BB : F) {
//     for (Instruction &I : BB) {
//         instructions.push_back(&I);
//     }}}

//     // replace collected instructions
//     for (auto it = instructions.begin(); it != instructions.end(); it++) {
//         LoadInst *LI = dyn_cast<LoadInst>(*it);
//         StoreInst *SI = dyn_cast<StoreInst>(*it);

//         if (LI) {
//             Type *T = LI->getType();
//             if (T->isPointerTy() || T == Int64Ty) continue;
//             Value *Ptr = LI->getPointerOperand();
//             Instruction *Ptr64 = BitCastInst::Create(Instruction::CastOps::BitCast, Ptr, Int64PtrTy, "", LI);
//             Instruction *NewLI = new LoadInst(Int64Ty, Ptr64, "", LI);
//             Instruction *Val = TruncInst::Create(Instruction::CastOps::Trunc, NewLI, T, "", LI);
//             LI->replaceAllUsesWith(Val);
//             LI->eraseFromParent();
//             LLVM_DEBUG(dbgs() << "STO64: replace to " << *NewLI << "\n");
//         }
//         else if (SI) {
//             Type *T = SI->getOperand(0)->getType();
//             if (T->isPointerTy() || T == Int64Ty) continue;
//             Value *Ptr = SI->getPointerOperand();
//             Value *Val = SI->getOperand(0);
//             Instruction *Ptr64 = BitCastInst::Create(Instruction::CastOps::BitCast, Ptr, Int64PtrTy, "", SI);
//             Instruction *Val64 = SExtInst::Create(Instruction::CastOps::SExt, Val, Int64Ty, "", SI);
//             Instruction *NewSI = new StoreInst(Val64, Ptr64, SI);
//             SI->eraseFromParent();
//             LLVM_DEBUG(dbgs() << "STO64: replace to " << *NewSI << "\n");
//         }
//     }
// }

// scale all malloc/alloca allocate size
static void scaleAllocation(Module &M, int multiplier) {
    vector<Instruction *> instructions;
    for (Function &F : M) {
    for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
        instructions.push_back(&I);
    }}}

    for (Instruction *I : instructions) {
        CallInst *CI = dyn_cast<CallInst>(I);
        AllocaInst *AI = dyn_cast<AllocaInst>(I);

        if (CI) {
            Function *F = CI->getCalledFunction();
            string name = F->getName().str();
            if (name == "malloc") {
                Value *SizeV = CI->getArgOperand(0);
                Instruction *DSizeV = BinaryOperator::Create(Instruction::BinaryOps::Mul, SizeV, ConstantInt::get(SizeV->getType(), multiplier));
                DSizeV->insertBefore(CI);
                CI->setArgOperand(0, DSizeV);
                LLVM_DEBUG(dbgs() << "STO64: replace to " << *CI << "\n");
            }
        }
        if (AI) {
            Type *T = AI->getAllocatedType();
            if (T->isArrayTy()) {
                ArrayType *AT = dyn_cast<ArrayType>(T);
                uint64_t num = AT->getNumElements();
                Type *NewAT = ArrayType::get(AT->getElementType(), AT->getNumElements() * multiplier);
                Instruction *NewAI = new AllocaInst(NewAT, 0, "", AI);

                // replace users arraytype, operand
                vector<Value *> users;
                for (auto it = AI->user_begin(); it != AI->user_end(); it++)
                    users.push_back(*it);
                for (auto it = users.begin(); it != users.end(); it++) {
                    Instruction *I = dyn_cast<Instruction>(*it);
                    GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(*it);
                    assert (GEP && "all alloca users should be GEP");
                    GEP->setSourceElementType(NewAT);
                    GEP->setOperand(0, NewAI);
                }

                AI->eraseFromParent();
                LLVM_DEBUG(dbgs() << "STO64: replace to " << *NewAI << "\n");
            }
        }
    }
}

// scale all GEP offset to load valid memory, only works when little endian
static void scaleGEPOffset(Module &M) {
    for (Function &F : M) {
    for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
        GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I);

        if (GEP) {
            Type *T = GEP->getType()->getPointerElementType();
            if (T->isPointerTy()) continue;
            unsigned int BW = T->getIntegerBitWidth();
            if (BW == 64) continue;
            unsigned int NumOperands = GEP->getNumOperands();
            unsigned int start = GEP->getOperand(0)->getType()->getPointerElementType()->isArrayTy() ? 2 : 1;
            for (unsigned int i = start; i < NumOperands; i++) {
                Value *OP = GEP->getOperand(i);
                Type *OPTy = OP->getType();
                Instruction *SOP = BinaryOperator::Create(Instruction::BinaryOps::Mul, OP, ConstantInt::get(OPTy, 64 / BW));
                SOP->insertBefore(GEP);
                GEP->setOperand(i, SOP);
            }
        }
    }}}
}

// multiply all global array size
static void scaleGlobalArray(Module &M, int multiplier) {
    vector<GlobalVariable *> globalVariables;
    for (auto it = M.global_begin(); it != M.global_end(); it++)
        globalVariables.push_back(&(*it));

    for (GlobalVariable *GV : globalVariables) {
        Type *T = GV->getValueType();
        if (T->isArrayTy()) {
            ArrayType *AT = dyn_cast<ArrayType>(T);

            // multiply the array type
            Type *NewAT = ArrayType::get(AT->getElementType(), AT->getNumElements() * multiplier);
            GlobalVariable *NewGV = new GlobalVariable(M, NewAT, false, GlobalValue::ExternalLinkage, nullptr);

            // replace users' arraytype, operand
            vector<Value *> users;
            for (Value *V : GV->users())
                users.push_back(V);
            for (Value *V : users) {
                GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(V);
                ConstantExpr *CE = dyn_cast<ConstantExpr>(V);

                if (GEP) {
                    GEP->setSourceElementType(NewAT);
                    GEP->setOperand(0, NewGV);
                }
                else if (CE) {
                    GetElementPtrInst *NewGEP = dyn_cast<GetElementPtrInst>(CE->getAsInstruction());
                    Value *Offset = NewGEP->getOperand(2);
                    Instruction *SO = BinaryOperator::Create(Instruction::BinaryOps::Mul, Offset, ConstantInt::get(Int64Ty, multiplier));
                    NewGEP->setOperand(0, NewGV);
                    NewGEP->setSourceElementType(NewAT);

                    vector<User *> CE_users;
                    for (auto user : CE->users()) CE_users.push_back(user);
                    for (auto user : CE_users) {
                        StoreInst *SI = dyn_cast<StoreInst>(user);
                        LoadInst *LI = dyn_cast<LoadInst>(user);
                        assert((LI || SI) && "all GV ConstantExpr users are Load/Store Inst");

                        Instruction *CopyGEP = NewGEP->clone();
                        Instruction *CopyDO = SO->clone();
                        CopyGEP->setOperand(2, CopyDO);
                        if (LI) {
                            LI->setOperand(0, CopyGEP);
                            CopyGEP->insertBefore(LI);
                        }
                        else {
                            SI->setOperand(1, CopyGEP);
                            CopyGEP->insertBefore(SI);
                        }
                        CopyDO->insertBefore(CopyGEP);
                    }
                    CE->destroyConstant();
                    NewGEP->deleteValue();
                    SO->deleteValue();
                }
            }
            GV->eraseFromParent();
            LLVM_DEBUG(dbgs() << "STO64: replace to " << NewGV << "\n");
        }
    }
}

PreservedAnalyses ScaleToInt64Pass::run(Module &M, ModuleAnalysisManager &MAM) {
    Int32Ty = Type::getInt32Ty(M.getContext());
    Int64Ty = Type::getInt64Ty(M.getContext());
    Int32PtrTy = Type::getInt32PtrTy(M.getContext());
    Int64PtrTy = Type::getInt64PtrTy(M.getContext());

    int bitwidth = getMinBitwidth(M);
    LLVM_DEBUG(dbgs() << "STO64: minimum bitwidth of i32 load/store - " << bitwidth << "\n";);
    LLVM_DEBUG(dbgs() << "STO64: number of global variables - " << M.global_size() << "\n";);

    if (bitwidth < 64) {
        // replaceLoadStores(M);
        scaleAllocation(M, 64 / bitwidth);
        scaleGEPOffset(M);
        scaleGlobalArray(M, 64 / bitwidth);
        return PreservedAnalyses::none();
    }
    else return PreservedAnalyses::all();
}

} // namespace team2_pass

#undef DEBUG_TYPE