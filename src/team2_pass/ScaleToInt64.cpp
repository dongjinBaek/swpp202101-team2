#include "ScaleToInt64.h"

using namespace std;
using namespace llvm;
using namespace team2_pass;

#define DEBUG_TYPE "scaletoi64"

namespace team2_pass {

static Type *Int32Ty, *Int64Ty, *Int32PtrTy, *Int64PtrTy;

// get minimum bitwidth of all loads/stores
static unsigned int getMinBitwidth(Module &M) {
    unsigned int bitwidth = 64;
    for (Function &F : M) for (BasicBlock &BB : F) for (Instruction &I : BB) {
        LoadInst *LI = dyn_cast<LoadInst>(&I);
        StoreInst *SI = dyn_cast<StoreInst>(&I);

        Type *T = nullptr;
        if (LI) T = LI->getType();
        else if (SI) T = SI->getOperand(0)->getType();
        if (!T || T->isPointerTy())
            continue;
        unsigned int b = T->getIntegerBitWidth();
        bitwidth = bitwidth < b ? bitwidth : b;
    }
    
    return bitwidth;
}

// scale all malloc/alloca allocate size
static void scaleMalloc(Module &M, unsigned int mul) {
    for (Function &F : M) for (BasicBlock &BB : F) for (Instruction &I : BB)
        if (CallInst *CI = dyn_cast<CallInst>(&I))
            if (CI->getCalledFunction()->getName() == "malloc") {
                Value *size = CI->getArgOperand(0);
                if (ConstantInt *Cst = dyn_cast<ConstantInt>(size))
                    CI->setArgOperand(0, ConstantInt::get(
                        IntegerType::getInt64Ty(M.getContext()),
                        Cst->getZExtValue() * mul));
                else {
                    Instruction *sizeI = BinaryOperator::CreateMul(size,
                        ConstantInt::get(size->getType(), mul));
                    sizeI->insertBefore(CI);
                    CI->setArgOperand(0, sizeI);
                    LLVM_DEBUG(dbgs() << "STO64: replace to " << *CI << "\n");
                }
            }
}

PreservedAnalyses ScaleToInt64Pass::run(Module &M, ModuleAnalysisManager &MAM) {
    unsigned int BW = getMinBitwidth(M);
    LLVM_DEBUG(dbgs() << "STO64: minimum bitwidth of load/store - " << BW << "\n";);

    if (BW < 64) {
        scaleMalloc(M, 64 / BW);
        return PreservedAnalyses::none();
    }
    return PreservedAnalyses::all();
}

} // namespace team2_pass

#undef DEBUG_TYPE
