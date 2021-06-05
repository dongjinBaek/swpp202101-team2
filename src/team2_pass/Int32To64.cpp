#include "Int32To64.h"

using namespace std;
using namespace llvm;
using namespace team2_pass;

#define DEBUG_TYPE "int32to64"

namespace team2_pass {

static Type *Int32Ty, *Int64Ty, *Int32PtrTy, *Int64PtrTy;

// find int32 load or store
static bool findInt32LoadStore(Module &M) {
    bool hasInt32 = false;
    for (Function &F : M) {
    for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
        LoadInst *LI = dyn_cast<LoadInst>(&I);
        StoreInst *SI = dyn_cast<StoreInst>(&I);

        if (LI && LI->getType() == Int32Ty)
            return true;
        else if (SI && SI->getOperand(0)->getType() == Int32Ty)
            return true;
    }}}
    return false;
}

// replace all i32 loads/stores to i64 loads/stores
static void replaceI32LoadStores(Module &M) {
    // collect instructions to replace, because iterator breaks when replacing
    vector<Instruction *> instructions;
    for (Function &F : M) {
    for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
        instructions.push_back(&I);
    }}}

    // replace collected instructions
    for (auto it = instructions.begin(); it != instructions.end(); it++) {
        LoadInst *LI = dyn_cast<LoadInst>(*it);
        StoreInst *SI = dyn_cast<StoreInst>(*it);

        if (LI && LI->getType() == Int32Ty) {
            Value *Ptr32 = LI->getPointerOperand();
            Instruction *Ptr64 = BitCastInst::Create(Instruction::CastOps::BitCast, Ptr32, Int64PtrTy, "", LI);
            Instruction *NewLI = new LoadInst(Int64Ty, Ptr64, "", LI);
            Instruction *Val32 = TruncInst::Create(Instruction::CastOps::Trunc, NewLI, Int32Ty, "", LI);
            LI->replaceAllUsesWith(Val32);
            LI->eraseFromParent();
            LLVM_DEBUG(dbgs() << "I32TO64: replace to " << *NewLI << "\n");
        }
        else if (SI && SI->getOperand(0)->getType() == Int32Ty) {
            Value *Ptr32 = SI->getPointerOperand();
            Value *Val32 = SI->getOperand(0);
            Instruction *Ptr64 = BitCastInst::Create(Instruction::CastOps::BitCast, Ptr32, Int64PtrTy, "", SI);
            Instruction *Val64 = SExtInst::Create(Instruction::CastOps::SExt, Val32, Int64Ty, "", SI);
            Instruction *NewSI = new StoreInst(Val64, Ptr64, SI);
            SI->eraseFromParent();
            LLVM_DEBUG(dbgs() << "I32TO64: replace to " << *NewSI << "\n");
        }
    }
}

// modify all malloc/alloca to allocate twice as much as original size
static void doubleAllocation(Module &M) {
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
                Instruction *DSizeV = BinaryOperator::Create(Instruction::BinaryOps::Mul, SizeV, ConstantInt::get(Int64Ty, 2));
                DSizeV->insertBefore(CI);
                CI->setArgOperand(0, DSizeV);
                LLVM_DEBUG(dbgs() << "I32TO64: replace to " << *CI << "\n");
            }
        }
        if (AI) {
            Type *T = AI->getAllocatedType();
            assert (T != Int32PtrTy && "T is int32ptrty");
            if (T->isArrayTy()) {
                ArrayType *AT = dyn_cast<ArrayType>(T);
                if (AT->getElementType() == Int32Ty) {
                    uint64_t num = AT->getNumElements();
                    Type *NewAT = ArrayType::get(Int64Ty, num * 2);
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
                    LLVM_DEBUG(dbgs() << "I32TO64: replace to " << *NewAI << "\n");
                }
            }
        }
    }
}

// double i32 GEP offset to load valid memory, only works when little endian
static void doubleGEPOffset(Module &M) {
    for (Function &F : M) {
    for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
        GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(&I);

        if (GEP && GEP->getType() == Int32PtrTy) {
            Value *OP;
            if (GEP->getNumOperands() == 2)
                OP = GEP->getOperand(1);
            else if (GEP->getNumOperands() == 3 && GEP->getOperand(1) == ConstantInt::get(Int64Ty, 0))
                OP = GEP->getOperand(2);
            else
                assert(false && "don't cover when GEP has more than 3 operands");
            
            Instruction *DOP = BinaryOperator::Create(Instruction::BinaryOps::Mul, OP, ConstantInt::get(Int64Ty, 2));
            DOP->insertBefore(GEP);
            GEP->setOperand(GEP->getNumOperands() - 1, DOP);
        }
    }}}
}

// double all i32 global array size
static void doubleI32GlobalArray(Module &M, ModuleAnalysisManager &MAM) {
    vector<GlobalVariable *> globalVariables;
    for (auto it = M.global_begin(); it != M.global_end(); it++)
        globalVariables.push_back(&(*it));

    for (GlobalVariable *GV : globalVariables) {
        Type *T = GV->getValueType();
        if (T->isArrayTy()) {
            ArrayType *AT = dyn_cast<ArrayType>(T);
            if (AT->getElementType() == Int32Ty) {
                // double the array type
                uint64_t num = AT->getNumElements();
                Type *NewAT = ArrayType::get(Int32Ty, num * 2);
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
                        Instruction *DO = BinaryOperator::Create(Instruction::BinaryOps::Mul, Offset, ConstantInt::get(Int64Ty, 2));
                        NewGEP->setOperand(0, NewGV);
                        NewGEP->setSourceElementType(NewAT);

                        vector<User *> CE_users;
                        for (auto user : CE->users()) CE_users.push_back(user);
                        for (auto user : CE_users) {
                            BitCastInst *BitCastI = dyn_cast<BitCastInst>(user);
                            assert(BitCastI && "all GV ConstantExpr users are BitCastI");

                            Instruction *CopyGEP = NewGEP->clone();
                            Instruction *CopyDO = DO->clone();
                            CopyGEP->insertBefore(BitCastI);
                            CopyGEP->setOperand(2, CopyDO);
                            CopyDO->insertBefore(CopyGEP);
                            BitCastI->setOperand(0, CopyGEP);
                        }
                        CE->destroyConstant();
                        NewGEP->deleteValue();
                        DO->deleteValue();
                    }
                }
                GV->eraseFromParent();
                LLVM_DEBUG(dbgs() << "I32TO64: replace to " << NewGV << "\n");
            }
        }
    }
}

PreservedAnalyses Int32To64Pass::run(Module &M, ModuleAnalysisManager &MAM) {
    Int32Ty = Type::getInt32Ty(M.getContext());
    Int64Ty = Type::getInt64Ty(M.getContext());
    Int32PtrTy = Type::getInt32PtrTy(M.getContext());
    Int64PtrTy = Type::getInt64PtrTy(M.getContext());

    bool found = findInt32LoadStore(M);
    LLVM_DEBUG(dbgs() << "I32TO64: found i32 load/store - " << found << "\n";);
    LLVM_DEBUG(dbgs() << "I32TO64: number of global variables - " << M.global_size() << "\n";);

    if (found) {
        replaceI32LoadStores(M);
        doubleAllocation(M);
        doubleGEPOffset(M);
        doubleI32GlobalArray(M, MAM);
        return PreservedAnalyses::none();
    }
    else return PreservedAnalyses::all();
}

} // namespace team2_pass

#undef DEBUG_TYPE