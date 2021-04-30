#include <sstream>
#include <iomanip>

#include "state.h"
#include "error.h"


CostStack::CostStack(const string &_fname): fname(_fname), cost(0), callees() {}

double CostStack::get_cost() const { return cost; }

void CostStack::add_cost(double _cost) { cost += _cost; }

void CostStack::set_callee(CostStack *callee) {
  callees.push_back(callee);
}

void CostStack::evaluate() {
  for (auto it: callees) {
    it->evaluate();
    cost += it->get_cost();
  }
}

string CostStack::to_string(const string& indent) const {
  stringstream ss;
  ss << fixed << setprecision(4);
  ss << indent << fname << ": " << cost << "\n";
  for (auto it: callees)
    ss << it->to_string(indent + "| ");
  return ss.str();
}


State::State(): regfile(), memory(), main_cost(nullptr), program(nullptr) {}

void State::set_program(Program* _program) {
  if (program == nullptr)
    program = _program;
}

double State::get_cost_value() const { return main_cost->get_cost(); }

CostStack * State::get_cost() const { return main_cost; }

uint64_t State::get_max_alloced_size() const {
  return memory.get_max_alloced_size();
}

uint64_t State::exec_function(CostStack* parent, Function* function) {
  auto cost = new CostStack(function->get_fname());
  if (parent == nullptr)
    main_cost = cost;
  else
    parent->set_callee(cost);

  Stmt* curr = function->get_first_bb();
  if (curr == nullptr)
    invoke_runtime_error("missing first basic block");

  while (true) {
    error_line_num = curr->get_line();

    switch (curr->get_opcode()) {
      case Ret: {
        auto stmt = dynamic_cast<StmtRet*>(curr);
        uint64_t ret = stmt->get_val().get_value(regfile);
        cost->add_cost(Cost::RET);
        _ret += Cost::RET;
        return ret;
      }
      case BrUncond: {
        auto stmt = dynamic_cast<StmtBrUncond*>(curr);
        string_view bb = stmt->get_bb();
        curr = function->get_bb(bb);
        if (curr == nullptr) {
          invoke_runtime_error("branching to an undefined basic block");
          return 0;
        }
        cost->add_cost(Cost::BRUNCOND);
        _bruncond += Cost::BRUNCOND;
        break;
      }
      case BrCond: {
        auto stmt = dynamic_cast<StmtBrCond*>(curr);
        string_view bb = stmt->get_bb(regfile);
        curr = function->get_bb(bb);
        if (curr == nullptr) {
          invoke_runtime_error("branching to an undefined basic block");
          return 0;
        }
        cost->add_cost(stmt->get_eval() ? Cost::BRCOND_TRUE : Cost::BRCOND_FALSE);
        if (stmt->get_eval()) _brcond_true  += Cost::BRCOND_TRUE;
        else                  _brcond_false += Cost::BRCOND_FALSE;
        break;
      }
      case Switch: {
        auto stmt = dynamic_cast<StmtSwitch*>(curr);
        string_view bb = stmt->get_bb(regfile);
        curr = function->get_bb(bb);
        if (curr == nullptr) {
          invoke_runtime_error("branching to an undefined basic block");
          return 0;
        }
        cost->add_cost(Cost::SWITCH);
        _switch += Cost::SWITCH;
        break;
      }
      case Call: {
        auto stmt = dynamic_cast<StmtCall*>(curr);
        string_view fname = stmt->get_fname();
        Function* callee = program->get_function(fname);
        if (callee == nullptr) {
          invoke_runtime_error("calling an undefined function");
          return 0;
        }

        int nargs = callee->get_nargs();
        if (nargs != stmt->get_nargs()) {
          invoke_runtime_error("calling a function with incorrect number of arguments");
          return 0;
        }

        RegFile old = regfile;

        regfile.set_nargs(nargs);
        stmt->setup_args(old, regfile);
        cost->add_cost(Cost::CALL + nargs * Cost::PER_ARG);
        _call += Cost::CALL;
        _call_arg += nargs * Cost::PER_ARG;
        uint64_t ret = exec_function(cost, callee);
        regfile = old;
        regfile.write_reg(curr->get_lhs(), ret);

        curr = stmt->get_next();
        break;
      }
      default: {
        cost->add_cost(curr->exec(regfile, memory));
        curr = curr->get_next();
      }
    }
    #ifdef USE_MEMORY_TEMP
    if (curr->get_opcode() != Assert)
      memory.advance_time();
    #endif // USE_MEMORY_TEMP
  }
}

uint64_t State::exec_program() {
  Function* main = program->get_function("main");
  if (main == nullptr)
    invoke_runtime_error("missing main function");
  uint64_t res = exec_function(nullptr, main);
  main_cost->evaluate();
  // memory.elapsed_times();
  return res;
}
