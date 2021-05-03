#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include "parser.h"
#include "state.h"
#include "error.h"

using namespace std;

double _ret, _bruncond, _brcond_true, _brcond_false, _switch, _call;
double _call_arg, _malloc, _free, _load_stack, _load_heap, _store_stack;
double _store_heap, _temp, _vload_stack, _vload_heap, _vstore_stack;
double _vstore_heap, _vtemp, _cool, _muldiv, _logical, _addsub, _comp;
double _ternary, _read, _write;

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "USAGE: sf-interpreter <input assembly file>" << endl;
    return 1;
  }

  string filename = argv[1];
  error_filename = filename;

  Program* program = parse(filename);
  if (program == nullptr) {
    cout << "Error: cannot find " << filename << endl;
    return 1;
  }

  State state;
  state.set_program(program);
  uint64_t ret = state.exec_program();

  ofstream log("sf-interpreter.log");
  log << fixed << setprecision(4);
  log << "Returned: " << ret << endl;
  log << "Cost: " << state.get_cost_value() << endl;
  log << "Max heap usage (bytes): " << state.get_max_alloced_size() << endl;
  log << endl;
  vector <pair<double, string>> v;
  v.emplace_back(_ret,          "ret:          ");
  v.emplace_back(_bruncond,     "bruncond:     ");
  v.emplace_back(_brcond_true,  "brcond_true:  ");
  v.emplace_back(_brcond_false, "brcond_false: ");
  v.emplace_back(_switch,       "switch:       ");
  v.emplace_back(_call,         "call:         ");
  v.emplace_back(_call_arg,     "call_arg:     ");
  v.emplace_back(_malloc,       "malloc:       ");
  v.emplace_back(_free,         "free:         ");
  v.emplace_back(_load_stack,   "load_stack:   ");
  v.emplace_back(_load_heap,    "load_heap:    ");
  v.emplace_back(_store_stack,  "store_stack:  ");
  v.emplace_back(_store_heap,   "store_heap:   ");
  v.emplace_back(_temp,         "temp:         ");
  v.emplace_back(_vload_stack,  "vload_stack:  ");
  v.emplace_back(_vload_heap,   "vload_heap:   ");
  v.emplace_back(_vstore_stack, "vstore_stack: ");
  v.emplace_back(_vstore_heap,  "vstore_heap:  ");
  v.emplace_back(_vtemp,        "vtemp:        ");
  v.emplace_back(_cool,         "cool:         ");
  v.emplace_back(_muldiv,       "muldiv:       ");
  v.emplace_back(_logical,      "logical:      ");
  v.emplace_back(_addsub,       "addsub:       ");
  v.emplace_back(_comp,         "comp:         ");
  v.emplace_back(_ternary,      "ternary:      ");
  v.emplace_back(_read,         "read:         ");
  v.emplace_back(_write,        "write:        ");
  sort(v.begin(), v.end(), greater<>());
  for (auto &data : v)
    log << data.second << data.first << endl;
  log.close();

  ofstream cost_log("sf-interpreter-cost.log");
  cost_log << state.get_cost()->to_string("");
  cost_log.close();

  return 0;
}
