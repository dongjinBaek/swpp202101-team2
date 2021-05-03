#ifndef SWPP_ASM_INTERPRETER_REG_H
#define SWPP_ASM_INTERPRETER_REG_H

#define NREGS 49


enum Reg {
  R1 = 0,
  R32 = 31,
  A1 = 32,
  A16 = 47,
  RegSp = 48,
  RegNone = 49
};

#endif //SWPP_ASM_INTERPRETER_REG_H
