#include "CondBranchDeflation.h"

/*******************************************************************************************
			CondBranchDeflationPass

 * Author: Seonghun Kim (zenith82114@gmail.com)
 
 * Attempt to reduce cost incurred by conditional branches.
 * If a block(BasicBlock) terminates with a conditional branch
	whose condition is an ICmp with no other use
	and whose true edge is repeatable while the false edge is not,
	(e.g. is part of a cycle(loop), leads to a block containing recursive call, etc.)
	invert the condition and swap its successor blocks;
 * if both edges are repeatable or neither,
	replace the branch with an equivalent switch instruction.

 *******************************************************************************************/
using namespace std;
using namespace llvm;
using namespace team2_pass;

namespace team2_pass {

vector<StringRef> v;	// visit stack
map<StringRef, bool> m;

/* recursively called on successors */
/* traverse the CFG depth-first, post-order */
/* modify the terminator when appropriate */
/* return true if the edge (BB's predecessor, BB) is repeatable */
static bool visitAndTransform(BasicBlock *BB) {
	//outs() << "Visiting " << BB->getName() << '\n';
	if (find(v.begin(), v.end(), BB->getName()) != v.end()) {		// visited twice; cycle detected; repeatable
		//outs() << "Found cycle\n";
		return true;
	}
	if (m.find(BB->getName()) != m.end()) {
		return m[BB->getName()];
	}
	bool hasRecursiveCall = false;
	for (auto &I : *BB) {
		CallInst *CI = dyn_cast<CallInst>(&I);
		if (!CI) continue;
		if (CI->getCalledFunction() == BB->getParent())				// recursive call; repeatable
			hasRecursiveCall = true;
	}

	Instruction *TI = dyn_cast<Instruction>(BB->getTerminator());
	assert(TI && "The terminator is an invalid instruction");

	if (isa<ReturnInst>(TI)) {										// terminates with ret:
		//outs() << "Return\n";
		//outs() << "Leaving " << BB->getName() << '\n';
		return m[BB->getName()] = hasRecursiveCall;									// not repeatable
	}
	if (isa<SwitchInst>(TI)) {										// terminates with switch:
		//outs() << "Switch\n";
		v.push_back(BB->getName());
		bool ret = false;
		for (unsigned s = 0; s < TI->getNumSuccessors(); ++s) {
			if (visitAndTransform(TI->getSuccessor(s)))				// repeatable iff at least one of its successors is
				ret = true;
		}
		assert(v.back() == BB->getName());
		v.pop_back();
		//outs() << "Leaving " << BB->getName() << '\n';
		return m[BB->getName()] = ret || hasRecursiveCall;
	}

	BranchInst *BI = dyn_cast<BranchInst>(TI);
	assert(BI && "The terminator does not have an allowed type");	// only four types of terminator (ret, switch, brC, brUC)

	if (BI->isUnconditional()) {									// terminates with brUC:
		//outs() << "UnCond\n";
		v.push_back(BB->getName());
		bool ret = visitAndTransform(BI->getSuccessor(0));
		assert(v.back() == BB->getName());
		v.pop_back();
		//outs() << "Leaving " << BB->getName() << '\n';
		return m[BB->getName()] = ret || hasRecursiveCall;								// repeatable iff its successor is
	}
	if (BI->isConditional()){										// terminates with brC:
		//outs() << "Cond\n";
		v.push_back(BB->getName());
		Value *V = BI->getCondition();
		BasicBlock *BBT = BI->getSuccessor(0);
		BasicBlock *BBF = BI->getSuccessor(1);
		bool mayRepeatTrue  = visitAndTransform(BBT);
		bool mayRepeatFalse = visitAndTransform(BBF);
		if (mayRepeatTrue == mayRepeatFalse) {						// both/neither edge repeatable:
			//outs() << "No tendency. Replace with Switch\n";
			IRBuilder<> builder(BI);
			IntegerType *i1type = IntegerType::getInt1Ty(BB->getContext());
			ConstantInt *C0 = ConstantInt::get(i1type, 0);
			SwitchInst *SI = builder.CreateSwitch(V, BBT, 1);		// replace with switch
			SI->addCase(C0, BBF);
			BI->eraseFromParent();
		}
		else if (mayRepeatTrue && !mayRepeatFalse) {				// only true edge is repeatable:
			//outs() << "True heavy\n";
			ICmpInst *ICI = dyn_cast<ICmpInst>(V);
			if (ICI) {												// the condition is an ICmp
				//outs() << "Cond is ICmp\n";
				bool one_use = true;
				for (auto u = V->use_begin(), end = V->use_end(); u != end;) {
					Use &U = *u++;
					Instruction *UsrI = dyn_cast<Instruction>(U.getUser());
					if (UsrI && (UsrI != BI)) {
						one_use = false;
						break;
					}
				}
				if (one_use) {											// ... and has no other use:
					//outs() << "Cond is used once. Invert and swap\n";
					ICI->setPredicate(ICI->getInversePredicate());		// invert condition
					BI->swapSuccessors();								// and swap successors
				}
				//else outs() << "Cond has other use\n";
			}
			//else outs() << "Cond is not ICmp. Skip\n";
		}
		assert(v.back() == BB->getName());
		v.pop_back();
		//outs() << "Leaving " << BB->getName() << '\n';
		return m[BB->getName()] = mayRepeatTrue || mayRepeatFalse || hasRecursiveCall;
	}
	//outs() << "Unreachable!!\n";
	return m[BB->getName()] = false;													// no other type of terminator; unreachable
}

PreservedAnalyses CondBranchDeflationPass::run(Module& M, ModuleAnalysisManager& MAM) {
	//outs() << "\n---------------CondBranchDeflationPass\n";
    for(Function& F : M) {
		//outs() << "---------- " << F.getName() << '\n';
		if (F.empty() && !F.isMaterializable())						// Skip functions that are declared but not defined (e.g. read, write)
			continue;
		v.clear();
		m.clear();
		BasicBlock& EB = F.getEntryBlock();
		visitAndTransform(&EB);
    }
    return PreservedAnalyses::all();
}

}