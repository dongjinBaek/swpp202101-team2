#ifndef SWPP_ASM_INTERPRETER_STATE_H
#define SWPP_ASM_INTERPRETER_STATE_H

#include <vector>

#include "regfile.h"
#include "memory.h"
#include "program.h"

using namespace std;


class CostStack {
private:
  string fname;
  double cost;
  vector<CostStack*> callees;

public:
  explicit CostStack(const string& _fname);
  double get_cost() const;
  void add_cost(double _cost);
  void set_callee(CostStack* callee);
  void evaluate();
  string to_string(const string& indent) const;
};


class State {
private:
  RegFile regfile;
  Memory memory;
  CostStack* main_cost;
  Program* program;

  uint64_t exec_function(CostStack* parent, Function* function);

public:
  State();

  void set_program(Program* _program);
  double get_cost_value() const;
  CostStack* get_cost() const;
  uint64_t get_max_alloced_size() const;
  uint64_t exec_program();
};

#endif //SWPP_ASM_INTERPRETER_STATE_H
