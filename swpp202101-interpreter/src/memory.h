#ifndef SWPP_ASM_INTERPRETER_MEMORY_H
#define SWPP_ASM_INTERPRETER_MEMORY_H

// enable/disable this macro to turn on/off the effect of temp
#define USE_MEMORY_TEMP

#include <cinttypes>
#include <limits>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>

#include "size.h"

#define STACK_MIN ((uint64_t)0)
#define STACK_MAX ((uint64_t)102400)
#define HEAP_MIN ((uint64_t)204800)
#define HEAP_MAX ((uint64_t)(numeric_limits<uint64_t>::max() - 7))

using namespace std;

typedef pair<uint64_t, uint64_t> block_t;
typedef pair<block_t, uint8_t*> alloc_t;
typedef uint64_t tick_t;


class Temperature {
public:
  const static int LINE_SIZE = 8;
private:
  const static int MIN_TEMP = 0;
  const static int MAX_TEMP = 200;
  const static int INC = 25;
  const static int DEC = 1;

  struct TempMapEntry {
    tick_t last_fetched;
    int temp;
  };
  unordered_map<uint64_t, TempMapEntry> temp_map;

  tick_t current_tick;

  static int inc_temp(int temp);
  static int dec_temp(int temp);

public:
  Temperature();

  int on_access(uint64_t ptr_addr); // return its old temperature
  void advance_tick();
  void freeze(uint64_t addr);
  int remove(uint64_t addr);
};


class Memory {
private:
  Temperature temp;
  uint8_t stack[STACK_MAX]{};
  set<alloc_t> alloced;
  set<block_t> freed;
  uint64_t alloced_size;
  uint64_t max_alloced_size;

  alloc_t find_block(uint64_t addr) const;
  uint64_t load_stack(MSize size, uint64_t addr) const;
  uint64_t load_heap(MSize size, uint64_t addr) const;
  void store_stack(MSize size, uint64_t addr, uint64_t val);
  void store_heap(MSize size, uint64_t addr, uint64_t val);

public:
  Memory();

  uint64_t get_alloced_size() const;
  uint64_t get_max_alloced_size() const;
  double exec_load(MSize size, uint64_t addr, uint64_t& result);
  double exec_store(MSize size, uint64_t addr, uint64_t val);
  double exec_vload(VSize size, uint64_t addr, uint64_t* results, const bool* mask);
  double exec_vstore(VSize size, uint64_t addr, uint64_t* vals, const bool* mask);
  double exec_malloc(uint64_t size, uint64_t& result);
  double exec_free(uint64_t addr);
  void exec_cool(uint64_t addr);
  void advance_time();
};

#endif //SWPP_ASM_INTERPRETER_MEMORY_H
