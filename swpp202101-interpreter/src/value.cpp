#include "value.h"


Value::Value(Reg _reg): kind(true), reg(_reg), literal(0) {}

Value::Value(uint64_t _literal): kind(false), reg(RegNone), literal(_literal) {}

Value::Value(const Value &val): kind(val.kind), reg(val.reg), literal(val.literal) {}

uint64_t Value::get_value(const RegFile& regfile) const {
  if (kind)
    return regfile.read_reg(reg);
  else
    return literal;
}

bool Value::is_reg_none() const {
  return kind && reg == RegNone;
}