#include <iostream>
#include <unordered_set>

#include "stmt.h"
#include "error.h"


Stmt::Stmt(int _line, Reg _lhs, Opcode _opcode): line(_line), lhs(_lhs), opcode(_opcode), next(nullptr) {}

int Stmt::get_line() const { return line; }

Reg Stmt::get_lhs() const { return lhs; }

Opcode Stmt::get_opcode() const { return opcode; }

Stmt *Stmt::get_next() const { return next; }

void Stmt::set_next(Stmt *stmt) { next = stmt; }


/** memory operations */

StmtMalloc::StmtMalloc(int _line, Reg _lhs, const Value& _val): Stmt(_line, _lhs, Malloc), val(_val) {}

double StmtMalloc::exec(RegFile &regfile, Memory &memory) const {
  uint64_t size = val.get_value(regfile);
  uint64_t addr;
  double cost = memory.exec_malloc(size, addr);
  regfile.write_reg(get_lhs(), addr);
  return cost;
}

StmtFree::StmtFree(int _line, const Value& _ptr): Stmt(_line, RegNone, Free), ptr(_ptr) {}

double StmtFree::exec(RegFile &regfile, Memory &memory) const {
  uint64_t addr = ptr.get_value(regfile);
  return memory.exec_free(addr);
}

StmtLoad::StmtLoad(int _line, Reg _lhs, MSize _size, const Value& _ptr, uint64_t _ofs):
Stmt(_line, _lhs, Load), size(_size), ptr(_ptr), ofs(_ofs) {}

double StmtLoad::exec(RegFile &regfile, Memory &memory) const {
  uint64_t addr = ptr.get_value(regfile) + ofs;
  uint64_t result;
  double cost = memory.exec_load(size, addr, result);
  regfile.write_reg(get_lhs(), result);
  return cost;
}

StmtStore::StmtStore(int _line, MSize _size, const Value& _val, const Value& _ptr, uint64_t _ofs):
Stmt(_line, RegNone, Store), size(_size), val(_val), ptr(_ptr), ofs(_ofs) {}

double StmtStore::exec(RegFile &regfile, Memory &memory) const {
  uint64_t addr = ptr.get_value(regfile) + ofs;
  uint64_t v = val.get_value(regfile);
  return memory.exec_store(size, addr, v);
}

StmtVLoad::StmtVLoad(int _line, VSize _size, const Reg *_lregs, const Value& _ptr, uint64_t _ofs):
Stmt(_line, RegNone, VLoad), size(_size), ptr(_ptr), ofs(_ofs) {
  std::unordered_set<Reg> lhs_registers;
  for (int i = 0; i < vsize_of(_size); i++) {
    Reg r = _lregs[i];
    if (r != RegNone && !lhs_registers.insert(r).second) {
      invoke_runtime_error("duplicate registers on left-hand side");
    }

    lregs[i] = r;
    mask[i] = r != RegNone;
  }
}

double StmtVLoad::exec(RegFile &regfile, Memory &memory) const {
  uint64_t addr = ptr.get_value(regfile) + ofs;
  uint64_t results[8];
  double cost = memory.exec_vload(size, addr, results, mask);
  for (int i = 0; i < vsize_of(size); i++) {
    if (mask[i])
      regfile.write_reg(lregs[i], results[i]);
  }
  return cost;
}

StmtVStore::StmtVStore(int _line, VSize _size, vector<Value> &_values, const Value& _ptr, uint64_t _ofs):
        Stmt(_line, RegNone, VStore), size(_size), ptr(_ptr), ofs(_ofs) {
  for (int i = 0; i < vsize_of(_size); i++) {
    values.emplace_back(_values.at(i));
    mask[i] = !_values.at(i).is_reg_none();
  }
}

double StmtVStore::exec(RegFile &regfile, Memory &memory) const {
  uint64_t addr = ptr.get_value(regfile) + ofs;
  uint64_t vals[8];
  for (int i = 0; i < vsize_of(size); i++) {
    if (mask[i])
      vals[i] = values.at(i).get_value(regfile);
  }
  return memory.exec_vstore(size, addr, vals, mask);
}

StmtVCool::StmtVCool(int _line, const Value& _ptr):
Stmt(_line, RegNone, Cool), ptr(_ptr) {}

double StmtVCool::exec(RegFile &regfile, Memory &memory) const {
  uint64_t addr = ptr.get_value(regfile);
  memory.exec_cool(addr);
  _cool += Cost::COOL;
  return Cost::COOL;
}


/** terminators */

StmtRet::StmtRet(int _line, Value _val): Stmt(_line, RegNone, Ret), val(_val) {}

Value StmtRet::get_val() const { return val; }

double StmtRet::exec(RegFile &regfile, Memory &memory) const { return 0; }

StmtBrUncond::StmtBrUncond(int _line, string _bb): Stmt(_line, RegNone, BrUncond), bb(move(_bb)) {}

string_view StmtBrUncond::get_bb() const { return bb; }

double StmtBrUncond::exec(RegFile &regfile, Memory &memory) const { return 0; }

StmtBrCond::StmtBrCond(int _line, Value _cond, string _true_bb, string _false_bb):
Stmt(_line, RegNone, BrCond), cond(_cond), true_bb(move(_true_bb)), false_bb(move(_false_bb)) {}

string_view StmtBrCond::get_bb(const RegFile& regfile) {
  uint64_t c = cond.get_value(regfile);
  if (c != 0) {
    eval = true;
    return true_bb;
  } else {
    eval = false;
    return false_bb;
  }
}

bool StmtBrCond::get_eval() const { return eval; }

double StmtBrCond::exec(RegFile &regfile, Memory &memory) const { return 0; }

StmtSwitch::StmtSwitch(int _line, Value _cond): Stmt(_line, RegNone, Switch), cond(_cond) {}

bool StmtSwitch::set_bb(uint64_t val, string bb) {
  auto it = bb_map.find(val);
  if (it != bb_map.end())
    return false;
  bb_map.insert(pair<uint64_t, string>(val, move(bb)));
  return true;
}

string_view StmtSwitch::get_bb(const RegFile& regfile) const {
  auto it = bb_map.find(cond.get_value(regfile));
  if (it == bb_map.end())
    return default_bb;
  return it->second;
}

void StmtSwitch::set_default(string bb) { default_bb = move(bb); }

bool StmtSwitch::case_exists(uint64_t val) const {
  auto it = bb_map.find(val);
  return it != bb_map.end();
}

double StmtSwitch::exec(RegFile &regfile, Memory &memory) const { return 0; }


/** binary operations */

StmtBop::StmtBop(int _line, Reg _lhs, BopKind _bop_kind, Value _val1, Value _val2, Size _size):
Stmt(_line, _lhs, Bop), bop_kind(_bop_kind), val1(_val1), val2(_val2), size(_size) {}

bool is_signed_op(BopKind bop_kind) {
  switch(bop_kind) {
    case Udiv:
    case Urem:
    case Mul:
    case Shl:
    case Lshr:
    case And:
    case Or:
    case Xor:
    case Add:
    case Sub:
    case Eq:
    case Ne:
    case Ugt:
    case Uge:
    case Ult:
    case Ule:
      return false;
    case Ashr:
    case Sdiv:
    case Srem:
    case Sgt:
    case Sge:
    case Slt:
    case Sle:
      return true;
  }
}

bool is_shift_op(BopKind bop_kind) {
  switch(bop_kind) {
    case Shl:
    case Lshr:
    case Ashr:
      return true;
    default:
      return false;
  }
}

uint64_t get_op1(BopKind bop_kind, Size size, uint64_t val) {
  if (is_signed_op(bop_kind)) {
    switch (size) {
      case Size1:
        if (val % 2 == 1)
          return -1;
        else
          return 0;
      case Size8:
        return (int8_t)val;
      case Size16:
        return (int16_t)val;
      case Size32:
        return (int32_t)val;
      case Size64:
        return val;
    }
  }
  else {
    switch (size) {
      case Size1:
        if (val % 2 == 1)
          return 1;
        else
          return 0;
      case Size8:
        return (uint8_t)val;
      case Size16:
        return (uint16_t)val;
      case Size32:
        return (uint32_t)val;
      case Size64:
        return val;
    }
  }
}

uint64_t get_op2(BopKind bop_kind, Size size, uint64_t val) {
  if (is_shift_op(bop_kind)) {
    return val % bw_of(size);
  }

  return get_op1(bop_kind, size, val);
}

uint64_t get_result(Size size, uint64_t val) {
  switch (size) {
    case Size1:
      return val % 2;
    case Size8:
      return (uint8_t)val;
    case Size16:
      return (uint16_t)val;
    case Size32:
      return (uint32_t)val;
    case Size64:
      return val;
  }
}

uint64_t bool_to_u64(bool b) {
  if (b)
    return 1;
  else
    return 0;
}

uint64_t StmtBop::compute(uint64_t op1, uint64_t op2) const {
  op1 = get_op1(bop_kind, size, op1);
  op2 = get_op2(bop_kind, size, op2);
  uint64_t result = 0;

  switch (bop_kind) {
    case Udiv:
      if (op2 == 0) {
        invoke_runtime_error("division by zero");
        return 0;
      }
      result = op1 / op2;
      break;
    case Sdiv:
      if (op2 == 0) {
        invoke_runtime_error("division by zero");
        return 0;
      }
      result = (int64_t) op1 / (int64_t) op2;
      break;
    case Urem:
      if (op2 == 0) {
        invoke_runtime_error("division by zero");
        return 0;
      }
      result = op1 % op2;
      break;
    case Srem:
      if (op2 == 0) {
        invoke_runtime_error("division by zero");
        return 0;
      }
      result = (int64_t) op1 % (int64_t) op2;
      break;
    case Mul:
      result = op1 * op2;
      break;
    case Shl:
      result = op1 << op2;
      break;
    case Lshr:
      result = op1 >> op2;
      break;
    case Ashr:
      result = (int64_t) op1 >> op2;
      break;
    case And:
      result = op1 & op2;
      break;
    case Or:
      result = op1 | op2;
      break;
    case Xor:
      result = op1 ^ op2;
      break;
    case Add:
      result = op1 + op2;
      break;
    case Sub:
      result = op1 - op2;
      break;
    case Eq:
      result = bool_to_u64(op1 == op2);
      break;
    case Ne:
      result = bool_to_u64(op1 != op2);
      break;
    case Ugt:
      result = bool_to_u64(op1 > op2);
      break;
    case Uge:
      result = bool_to_u64(op1 >= op2);
      break;
    case Ult:
      result = bool_to_u64(op1 < op2);
      break;
    case Ule:
      result = bool_to_u64(op1 <= op2);
      break;
    case Sgt:
      result = bool_to_u64((int64_t) op1 > (int64_t) op2);
      break;
    case Sge:
      result = bool_to_u64((int64_t) op1 >= (int64_t) op2);
      break;
    case Slt:
      result = bool_to_u64((int64_t) op1 < (int64_t) op2);
      break;
    case Sle:
      result = bool_to_u64((int64_t) op1 <= (int64_t) op2);
      break;
  }

  return get_result(size, result);
}

double cost_of(BopKind bop_kind) {
  switch (bop_kind) {
    case Udiv:
    case Sdiv:
    case Urem:
    case Srem:
    case Mul:
      _muldiv += Cost::MULDIV;
      return Cost::MULDIV;
    case Shl:
    case Lshr:
    case Ashr:
    case And:
    case Or:
    case Xor:
      _logical += Cost::LOGICAL;
      return Cost::LOGICAL;
    case Add:
    case Sub:
      _addsub += Cost::ADDSUB;
      return Cost::ADDSUB;
    case Eq:
    case Ne:
    case Ugt:
    case Uge:
    case Ult:
    case Ule:
    case Sgt:
    case Sge:
    case Slt:
    case Sle:
      _comp += Cost::COMP;
      return Cost::COMP;
  }
}

double StmtBop::exec(RegFile& regfile, Memory& memory) const {
  uint64_t op1 = val1.get_value(regfile);
  uint64_t op2 = val2.get_value(regfile);
  uint64_t res = compute(op1, op2);
  regfile.write_reg(get_lhs(), res);
  return cost_of(bop_kind);
}


/** ternary operation */

StmtSelect::StmtSelect(int _line, Reg _lhs, Value _cond, Value _val_true, Value _val_false):
Stmt(_line, _lhs, Select), cond(_cond), val_true(_val_true), val_false(_val_false) {}

double StmtSelect::exec(RegFile& regfile, Memory& memory) const {
  uint64_t v_cond = cond.get_value(regfile);
  uint64_t v_true = val_true.get_value(regfile);
  uint64_t v_false = val_false.get_value(regfile);

  if (v_cond != 0)
    regfile.write_reg(get_lhs(), v_true);
  else
    regfile.write_reg(get_lhs(), v_false);

  _ternary += Cost::TERNARY;
  return Cost::TERNARY;
}


/** function call */

StmtCall::StmtCall(int _line, Reg _lhs, string _fname): Stmt(_line, _lhs, Call), fname(move(_fname)) {}

string_view StmtCall::get_fname() const { return fname; }

void StmtCall::push_arg(const Value arg) { args.push_back(arg); }

int StmtCall::get_nargs() { return args.size(); }

void StmtCall::setup_args(const RegFile &old, RegFile &regfile) {
  int r = (int)A1;
  for (auto it: args) {
    uint64_t val = it.get_value(old);
    regfile.set_value((Reg)r, val);
    r++;
  }
}

double StmtCall::exec(RegFile &regfile, Memory &memory) const { return 0; }


/** assertion */

StmtAssert::StmtAssert(int _line, Value _op1, Value _op2):
Stmt(_line, RegNone, Assert), op1(_op1), op2(_op2){}

double StmtAssert::exec(RegFile &regfile, Memory &memory) const {
  uint64_t val1 = op1.get_value(regfile);
  uint64_t val2 = op2.get_value(regfile);

  if (val1 == val2)
    return Cost::ASSERT;

  invoke_assertion_failed(regfile);
  return 0;
}


/** read and write */

StmtRead::StmtRead(int _line, Reg _lhs): Stmt(_line, _lhs, Read) {}

double StmtRead::exec(RegFile &regfile, Memory &memory) const {
  string input;
  cin >> input;

  try {
    uint64_t result = stoull(input);
    regfile.write_reg(get_lhs(), result);
    _read += Cost::CALL;
    return Cost::CALL;
  } catch (exception& e) {
    invoke_runtime_error("invalid input");
    return 0;
  }
}

StmtWrite::StmtWrite(int _line, Reg _lhs, Value _val): Stmt(_line, _lhs, Write), val(_val) {}

double StmtWrite::exec(RegFile &regfile, Memory &memory) const {
  uint64_t result = val.get_value(regfile);
  cout << result << "\n";
  regfile.write_reg(get_lhs(), 0);
  _write += Cost::CALL + Cost::PER_ARG;
  return Cost::CALL + Cost::PER_ARG;
}
