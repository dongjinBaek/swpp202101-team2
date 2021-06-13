#include "Backend.h"

using namespace std;
using namespace llvm;
using namespace backend;

namespace backend {

string AssemblyEmitter::name(Value* v) {
    if(!v || isa<ConstantPointerNull>(v) || v->getType()->isVoidTy()) {
        return "0";
    }
    if(isa<ConstantInt>(v)) {
        //return the value itself.
        return to_string(dyn_cast<ConstantInt>(v)->getZExtValue());
    }
    Symbol *symbol = SM->get(v);
    if (Memory *mem = symbol->castToMemory()) {
        if (mem->getBase() == TM->gvp())
            return to_string(204800 + mem->getOffset());
        else if (mem->getBase() == TM->sgvp())
            return to_string(102400 - mem->getOffset());
        else if (mem->getBase() == TM->sp() && mem->getOffset() == 0)
            return "sp";
    }
    return SM->get(v)->getName();
}

//static functions for emitting common formats.
string AssemblyEmitter::emitInst(vector<string> printlist) {
    string str = "  ";
    for(string s : printlist) {
        str += s + " ";
    }
    str += "\n";
    return str;
}
string AssemblyEmitter::emitBinary(Instruction* v, string opcode, string op1, string op2) {
    return emitInst({name(v), "=", opcode, op1, op2, stringBandWidth(v)});
}
string AssemblyEmitter::emitCopy(Instruction* v, Value* op) {
    Memory* mem = SM->get(op)? SM->get(op)->castToMemory() : NULL;
    if(mem) {
        if(mem->getBase() == TM->gvp()) {
            return emitBinary(v, "add", "204800", to_string(mem->getOffset()));    
        }
        else if (mem->getBase() == TM->sgvp())
            return emitBinary(v, "sub", "102400", to_string(mem->getOffset()));
        else if (mem->getBase() == TM->sp())
            return emitBinary(v, "add", v->getFunction()->getName() == "main" ?
                                "r32" : "sp", to_string(mem->getOffset()));
        return emitBinary(v, "add", mem->getBase()->getName(), to_string(mem->getOffset()));
    }
    return emitBinary(v, "mul", name(op), "1");
}

string AssemblyEmitter::stringBandWidth(Value* v) {
    if(isa<Function>(v) || isa<BasicBlock>(v)) {
        assert(false && "v should be a digit-typed value");
    }
    return to_string(getBitWidth(v->getType()));
}

AssemblyEmitter::AssemblyEmitter(raw_ostream *fout, TargetMachine& TM, SymbolMap& SM, map<Function*, unsigned>& spOffset) :
            fout(fout), TM(&TM), SM(&SM), spOffset(spOffset) {}

void AssemblyEmitter::visitFunction(Function& F) {
    //print the starting code.
    //finishing code will be printed outside the AssemblyEmitter.
    *fout << "start " << name(&F) << " " << F.arg_size() << ":\n";
}
void AssemblyEmitter::visitBasicBlock(BasicBlock& BB) {
    *fout << "." << name(&BB) << ":\n";

    //If entry block, modify SP.
    if(&(BB.getParent()->getEntryBlock()) == &BB) {
        //if main, import GV within.
        //this code should happen only if GV array was in the initial program.
        //GV values are all lowered into alloca + calls
        if(BB.getParent()->getName() == "main" && BB.getModule()->getGlobalList().size()!=0) {
            *fout << "  ; Init global variables\n";
            
            Module::global_iterator it;
            Module *M = BB.getModule();
            unsigned stack_acc = 0;

            // find separting point between global variables in stack and heap
            for (it = M->global_begin(); it != M->global_end(); it++) {
                GlobalVariable& gv = *it;
                if (gv.getName().contains('$')) continue;
                unsigned size = (getAccessSize(gv.getValueType()) + 7) / 8 * 8;
                if (stack_acc + size > MAX_STACK_SIZE) break;
                stack_acc += size;
            }
            
            // securing stack space
            if (stack_acc)
                *fout << emitInst({"sp = sub sp", to_string(stack_acc), "64"});
            
            // initialize GV in stack
            for (auto it2 = M->global_begin(); it2 != it; it2++) {
                GlobalVariable& gv = *it2;
                if (gv.getName().contains('$')) continue;
                unsigned size = (getAccessSize(gv.getValueType()) + 7) / 8 * 8;
                if (gv.hasInitializer() && !gv.getInitializer()->isZeroValue())
                    *fout << emitInst({"store", to_string(getAccessSize(
                        gv.getValueType())), name(gv.getInitializer()),
                        to_string(102400 - stack_acc), 0});
                stack_acc -= size;
            }

            // securing heap space + initialize GV in heap
            for(auto it2 = it; it2 != M->global_end(); it2++) {
                GlobalVariable& gv = *it2;
                if (gv.getName().contains('$')) continue;
                //temporarily stores the GV pointer.
                unsigned size = (getAccessSize(gv.getValueType()) + 7) / 8 * 8;
                *fout << emitInst({"r1 = malloc", to_string(size)});
                if(gv.hasInitializer() && !gv.getInitializer()->isZeroValue())
                    *fout << emitInst({"store", to_string(getAccessSize(
                        gv.getValueType())), name(gv.getInitializer()), "r1 0"});
            }
        }
        if(spOffset[BB.getParent()] != 0) {
            *fout << "  ; Init stack pointer\n";
            *fout << emitInst({"sp = sub sp",to_string(spOffset[BB.getParent()]),"64"});
            if (BB.getParent()->getName() == "main")
                *fout << emitInst({"r32 = mul sp 1 64"});
        }
    }
}

//Compare insts.
void AssemblyEmitter::visitICmpInst(ICmpInst& I) {
    *fout << emitInst({name(&I), "= icmp", I.getPredicateName(I.getPredicate()).str(), name(I.getOperand(0)), name(I.getOperand(1)), stringBandWidth(I.getOperand(0))});
}

//Alloca inst.
void AssemblyEmitter::visitAllocaInst(AllocaInst& I) {
    //Do nothing.
}

//Memory Access insts.
void AssemblyEmitter::visitLoadInst(LoadInst& I) {
    Value* ptr = I.getPointerOperand();
    if (ptr->getName().contains('$')) return;
    //bytes to load
    string size = to_string(getAccessSize(dyn_cast<PointerType>(ptr->getType())->getElementType()));
    Symbol* symbol = SM->get(ptr);
    //if pointer operand is a memory value(GV or alloca),
    if(Memory* mem = symbol->castToMemory()) {
        if(mem->getBase() == TM->sp()) {
            string base = I.getParent()->getParent()->getName() == "main" ? "r32" : "sp";
            *fout << emitInst({name(&I), "= load", size, base, to_string(mem->getOffset())});
        }
        else if(mem->getBase() == TM->gvp()) {
            *fout << emitInst({name(&I), "= load", size, "204800", to_string(mem->getOffset())});
        }
        else if (mem->getBase() == TM->sgvp())
            *fout << emitInst({name(&I), "= load", size, to_string(102400 - mem->getOffset()), "0"});
        else assert(false && "base of memory pointers should be sp or gvp");
    }
    //else a pointer stored in register,
    else if(Register* reg = symbol->castToRegister()) {
        *fout << emitInst({name(&I), "= load", size, reg->getName(), "0"});
    }
    else assert(false && "pointer of a memory operation should have an appropriate symbol assigned");
}
void AssemblyEmitter::visitStoreInst(StoreInst& I) {
    Value* ptr = I.getPointerOperand();
    //bytes to load
    string size = to_string(getAccessSize(dyn_cast<PointerType>(ptr->getType())->getElementType()));
    Value* val = I.getValueOperand();
    Symbol* symbol = SM->get(ptr);
    //if pointer operand is a memory value(GV or alloca),
    if(Memory* mem = symbol->castToMemory()) {
        if(mem->getBase() == TM->sp()) {
            string base = I.getParent()->getParent()->getName() == "main" ? "r32" : "sp";
            *fout << emitInst({"store", size, name(val), base, to_string(mem->getOffset())});
        }
        else if(mem->getBase() == TM->gvp()) {
            *fout << emitInst({"store", size, name(val), "204800", to_string(mem->getOffset())});
        }
        else if (mem->getBase() == TM->sgvp())
            *fout << emitInst({"store", size, name(val), to_string(102400 - mem->getOffset()), "0"});
        else assert(false && "base of memory pointers should be sp or gvp");
    }
    //else a pointer stored in register,
    else if(Register* reg = symbol->castToRegister()) {
        *fout << emitInst({"store", size, name(val),reg->getName(), "0"});
    }
    else assert(false && "pointer of a memory operation should have an appropriate symbol assigned");
}

//PHI Node inst.
void AssemblyEmitter::visitPHINode(PHINode& I) {
    //Do nothing.
}

//Reformatting(no value changes) insts.
void AssemblyEmitter::visitTruncInst(TruncInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitZExtInst(ZExtInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitSExtInst(SExtInst& I) {
    unsigned beforeBits = getBitWidth(I.getOperand(0)->getType());
    unsigned afterBits = getBitWidth(I.getType());
    assert(afterBits > beforeBits && "SExt must increase the bandwidth");
    *fout << emitBinary(&I, "mul", name(I.getOperand(0)), to_string(1llu<<(afterBits-beforeBits)));
    *fout << emitBinary(&I, "sdiv", name(&I), to_string(1llu<<(afterBits-beforeBits)));
}
void AssemblyEmitter::visitPtrToIntInst(PtrToIntInst& I) {
    if(SM->get(&I) != SM->get(I.getPointerOperand()))
        *fout << emitCopy(&I, I.getPointerOperand());
}
void AssemblyEmitter::visitIntToPtrInst(IntToPtrInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}
void AssemblyEmitter::visitBitCastInst(BitCastInst& I) {
    //If coallocated to the same registers, do nothing.
    //Else, copy the value.
    if(SM->get(&I) != SM->get(I.getOperand(0))) {
        *fout << emitCopy(&I, I.getOperand(0));
    }
}

//Select inst.
void AssemblyEmitter::visitSelectInst(SelectInst& I) {
    *fout << emitInst({name(&I), "= select", name(I.getCondition()), name(I.getTrueValue()), name(I.getFalseValue())});
}

void AssemblyEmitter::visitCallInst(CallInst& I) {
    //Process malloc()&free() from other plain call insts.
    Function* F = I.getCalledFunction();
    string Fname = F->getName().str();
    
    //Collect all arguments
    vector<string> args;
    for(Use& arg : I.args()) {
        args.push_back(name(arg.get()));
    }
    if(Fname == "malloc") {
        assert(args.size()==1 && "argument of malloc() should be 1");
        *fout << emitInst({name(&I), "= malloc", name(I.getArgOperand(0))});
    }
    else if (Fname == "$dyn_alloca") {
        assert(args.size()==2 && "argument of $dyn_alloca() should be 2");
        string name0 = name(I.getArgOperand(0)), name1 = name(I.getArgOperand(1));
        static unsigned int num = 0;
        string str = to_string(num++);
        // allocate on stack when sp - name0 >= stack_lower_bound
        string stack_lower_bound = to_string(102400 - MAX_STACK_SIZE);
        *fout << emitInst({name1, "= add", name0, stack_lower_bound, "64"});
        *fout << emitInst({name1, "= icmp ult sp", name1, "64"});
        *fout << emitInst({"br", name1, ".__sp.then" + str, ".__sp.else" + str});
        *fout << ".__sp.then" + str << ":\n";
        *fout << emitInst({name(&I), "= malloc", name0});
        *fout << emitInst({"br .__sp.next" + str});
        *fout << ".__sp.else" + str << ":\n";
        *fout << emitInst({"sp = sub sp", name0, "64"});
        *fout << emitInst({name(&I), "= mul sp 1 64"});
        *fout << emitInst({"br .__sp.next" + str});
        *fout << ".__sp.next" + str << ":\n";
    }
    else if (Fname == "$store") {
        assert(args.size()==3 && "argument of $store() should be 3");
        string name0 = name(I.getArgOperand(0)), name1 = name(I.getArgOperand(1));
        ConstantInt *CI = dyn_cast<ConstantInt>(I.getOperand(2));
        assert(CI && "%store bitwidth not ConstantInt");
        string width = to_string(CI->getZExtValue());
        *fout << emitInst({name0, "= mul 1", name1, width});
    }
    else if(Fname == "free") {
        assert(args.size()==1 && "argument of free() should be 1");
        *fout << emitInst({"free", name(I.getArgOperand(0))});
    }
    else if(UnfoldVectorInstPass::VLOADS.find(Fname) != UnfoldVectorInstPass::VLOADS.end()) {
        vector<string> asmb;
        int n = atoi(Fname.substr(Fname.size() - 1, 1).c_str());
        for (int i=0; i<n; i++) asmb.push_back("_");

        auto it = I.getIterator();
        for (it++; ;) {
            CallInst *NI = dyn_cast<CallInst>(&*it++);
            if (NI == NULL) break;

            string niFn = NI->getCalledFunction()->getName().str();
            if (niFn != "extract_element2" && niFn != "extract_element4" && niFn != "extract_element8") break;

            ConstantInt *C = dyn_cast<ConstantInt>(NI->getOperand(1));
            assert(C != NULL && "extract_element should retrieve a constant argument as a dim");

            int pos = C->getZExtValue();
            assert(0 <= pos && pos < asmb.size());

            asmb[pos] = name(NI);
        }

        asmb.push_back("= vload");
        asmb.push_back(Fname.substr(Fname.size() - 1, 1)); // n
        asmb.push_back(name(I.getOperand(0))); // ptr
        asmb.push_back("0"); // offset = 0

        *fout << emitInst(asmb);
    }
    else if(UnfoldVectorInstPass::VSTORES.find(Fname) != UnfoldVectorInstPass::VSTORES.end()) {
        vector<string> asmb;
        int n = atoi(Fname.substr(Fname.size() - 1, 1).c_str());

        asmb.push_back("vstore");
        asmb.push_back(Fname.substr(Fname.size() - 1, 1)); // n
        for (int i=0; i<n; i++) asmb.push_back("_");

        auto *maskVal = dyn_cast<ConstantInt>(I.getOperand(n + 1));
        assert(maskVal != NULL && "mask should be a constant integer");

        int mask = maskVal->getZExtValue();
        assert(0 <= mask && mask < (1 << n) && "invalid mask number");

        for (int i=0; i<n; i++) {
            if (!(mask & (1 << i))) continue;

            Value *V = I.getOperand(i);
            asmb[2+i] = name(V);
        }

        asmb.push_back(name(I.getOperand(n))); // ptr
        asmb.push_back("0"); // offset = 0

        *fout << emitInst(asmb);
    }
    else if(UnfoldVectorInstPass::EXTRACT_ELEMENTS.find(Fname) != UnfoldVectorInstPass::EXTRACT_ELEMENTS.end()) {
        // do nothing
    }
	else if(F->getReturnType()->isVoidTy()) {
		vector<string> printlist = {"call", Fname};
		printlist.insert(printlist.end(), args.begin(), args.end());
		*fout << emitInst(printlist);
	}
    //ordinary function calls.
    else {
        vector<string> printlist = {name(&I), "= call", Fname};
        printlist.insert(printlist.end(), args.begin(), args.end());
        *fout << emitInst(printlist);
    }
}

//Terminator insts.
void AssemblyEmitter::visitReturnInst(ReturnInst& I) {
    //increase sp(which was decreased in the beginning of the function.)
    Function* F = I.getFunction();
    if(spOffset[F] > 0) {
        *fout << emitInst({"sp = add sp",to_string(spOffset[F]),"64"});
    }
    *fout << emitInst({"ret", name(I.getReturnValue())});
}
void AssemblyEmitter::visitBranchInst(BranchInst& I) {
    if(I.isConditional()) {
        assert(I.getNumSuccessors() == 2 && "conditional branches must have 2 successors");
        *fout << emitInst({"br", name(I.getCondition()), "." + name(I.getSuccessor(0)), "." + name(I.getSuccessor(1))});
    }
    else {
        assert(I.getNumSuccessors() == 1 && "unconditional branches must have 1 successor");
        *fout << emitInst({"br", "." + name(I.getSuccessor(0))});
    }
}
void AssemblyEmitter::visitSwitchInst(SwitchInst& I) {
    string asmb("switch " + name(I.getCondition()));
    for(auto& c : I.cases()) {
        if(c.getCaseIndex() == I.case_default()->getCaseIndex()) continue;
        asmb.append(" " + name(c.getCaseValue()) + " ." + name(c.getCaseSuccessor()));
    }
    asmb.append(" ." + name(I.case_default()->getCaseSuccessor()));
    *fout << asmb << "\n";
}
void AssemblyEmitter::visitBinaryOperator(BinaryOperator& I) {
    string opcode = "";
    switch(I.getOpcode()) {
    case Instruction::UDiv: opcode = "udiv"; break;
    case Instruction::SDiv: opcode = "sdiv"; break;
    case Instruction::URem: opcode = "urem"; break;
    case Instruction::SRem: opcode = "srem"; break;
    case Instruction::Mul:  opcode = "mul"; break;
    case Instruction::Shl:  opcode = "shl"; break;
    case Instruction::AShr: opcode = "ashr"; break;
    case Instruction::LShr: opcode = "lshr"; break;
    case Instruction::And:  opcode = "and"; break;
    case Instruction::Or:   opcode = "or"; break;
    case Instruction::Xor:  opcode = "xor"; break;
    case Instruction::Add:  opcode = "add"; break;
    case Instruction::Sub:  opcode = "sub"; break;
    default: assert(false && "undefined binary operation");
    }

    *fout << emitBinary(&I, opcode, name(I.getOperand(0)), name(I.getOperand(1)));
}

}
