#include <cstring>

#include "error.h"
#include "opcode.h"
#include "memory.h"


/** Temperature */

Temperature::Temperature(): temp_map(), current_tick(0) {}

int Temperature::inc_temp(int temp) {
  temp += INC;
  return temp <= MAX_TEMP ? temp : MAX_TEMP;
}

int Temperature::dec_temp(int temp) {
  temp -= DEC;
  return temp >= MIN_TEMP ? temp : MIN_TEMP;
}

uint64_t translate_heat_address(uint64_t addr) {
  return addr - addr % Temperature::LINE_SIZE;
}

int Temperature::on_access(uint64_t ptr_addr) {
#ifdef USE_MEMORY_TEMP
  uint64_t segment_addr_start = translate_heat_address(ptr_addr);

  auto res = temp_map.emplace(
    make_pair(segment_addr_start, TempMapEntry{current_tick, 0}));
  auto itr = res.first;
  int temp_curr = 0;

  if (!res.second) {
    // not first access to temp_map
    tick_t last_accessed_tick = itr->second.last_fetched;
    tick_t passed_ticks = current_tick - last_accessed_tick;
    // memory access prevents global cooldown for that tick, so decrement 1
    int temp_to_reduce = ((int)passed_ticks - 1) * DEC;

    temp_curr = max(0, itr->second.temp - temp_to_reduce);
  }

  itr->second.temp = inc_temp(itr->second.temp);
  itr->second.last_fetched = current_tick;
  return temp_curr;
#else
  return 0;
#endif // USE_MEMORY_TEMP
}

void Temperature::advance_tick() {
  current_tick += 1;
}

void Temperature::freeze(uint64_t addr) {
  auto itr = temp_map.find(addr);
  if (itr == temp_map.end())
    return;
  itr->second.temp = 0;
}

int Temperature::remove(uint64_t addr) {
  return temp_map.erase(addr);
}


/** Memory */

Memory::Memory(): temp(), alloced(), freed(), alloced_size(0), max_alloced_size(0) {
  memset(stack, 0, STACK_MAX);
  freed.insert(block_t(HEAP_MIN, HEAP_MAX));
}

alloc_t Memory::find_block(uint64_t addr) const {
  auto lb = alloced.lower_bound(alloc_t(block_t(addr, -1), nullptr));

  if (lb == alloced.begin()) {
    invoke_runtime_error("accessing non-allocated memory");
  }

  lb--;
  if (lb->first.second <= addr) {
    invoke_runtime_error("accessing non-allocated memory");
  }

  return *lb;
}

uint64_t load_little_endian(int size, uint8_t* ptr) {
  uint64_t sum = 0;
  ptr = ptr + size;

  for (int i = 0; i < size; i++) {
    sum = sum << (uint64_t)8;
    ptr--;
    sum += *ptr;
  }

  return sum;
}

uint64_t Memory::load_stack(MSize size, uint64_t addr) const {
  auto ptr = (uint8_t*)&stack[0];
  return load_little_endian(msize_of(size), ptr + addr);
}

uint64_t Memory::load_heap(MSize size, uint64_t addr) const {
  alloc_t block = find_block(addr);
  uint64_t start = block.first.first;
  uint64_t ofs = addr - start;
  uint8_t* ptr = block.second + ofs;

  return load_little_endian(msize_of(size), ptr);
}

void store_little_endian(int size, uint8_t* ptr, uint64_t val) {
  uint64_t mask = 0xFF;
  for (int i = 0; i < size; i++) {
    *ptr = val & mask;
    ptr++;
    val = val >> (uint64_t)8;
  }
}

void Memory::store_stack(MSize size, uint64_t addr, uint64_t val) {
  auto ptr = (uint8_t*)&stack[0];
  return store_little_endian(msize_of(size), ptr + addr, val);
}

void Memory::store_heap(MSize size, uint64_t addr, uint64_t val) {
  alloc_t block = find_block(addr);
  uint64_t start = block.first.first;
  uint64_t ofs = addr - start;
  uint8_t* ptr = block.second + ofs;

  store_little_endian(msize_of(size), ptr, val);
}

bool is_stack(MSize size, uint64_t addr) {
  return addr + msize_of(size) <= STACK_MAX;
}

bool is_heap(MSize size, uint64_t addr) {
  return HEAP_MIN <= addr && addr + msize_of(size) <= HEAP_MAX;
}

bool is_alligned(MSize size, uint64_t addr) {
  return addr % msize_of(size) == 0;
}

double stack_cost(int temperature) {
#ifdef USE_MEMORY_TEMP
  return Cost::STACK + Cost::PER_TEMP * temperature;
#else
  return Cost::STACK;
#endif // USE_MEMORY_TEMP
}

double heap_cost(int temperature) {
#ifdef USE_MEMORY_TEMP
  return Cost::HEAP + Cost::PER_TEMP * temperature;
#else
  return Cost::HEAP;
#endif // USE_MEMORY_TEMP
}

double vstack_cost(int temperature) {
#ifdef USE_MEMORY_TEMP
  return Cost::VSTACK + Cost::PER_TEMP * temperature;
#else
  return Cost::VSTACK;
#endif // USE_MEMORY_TEMP
}

double vheap_cost(int temperature) {
  #ifdef USE_MEMORY_TEMP
  return Cost::VHEAP + Cost::PER_TEMP * temperature;
  #else
  return Cost::VHEAP;
  #endif // USE_MEMORY_TEMP
}

double Memory::exec_load(MSize size, uint64_t addr, uint64_t& result) {
  if (!is_alligned(size, addr)) {
    invoke_runtime_error("address not aligned");
    return 0;
  }

  double cost;
  int temp_old = this->temp.on_access(addr);

  if (is_stack(size, addr)) {
    result = load_stack(size, addr);
    cost = stack_cost(temp_old);
    _load_stack += Cost::STACK;
    _temp += Cost::PER_TEMP * temp_old;
  } else if (is_heap(size, addr)) {
    result = load_heap(size, addr);
    cost = heap_cost(temp_old);
    _load_heap += Cost::HEAP;
    _temp += Cost::PER_TEMP * temp_old;
  } else {
    invoke_runtime_error("accessing address between 102400 and 204800");
    return 0;
  }
  return cost;
}

double Memory::exec_store(MSize size, uint64_t addr, uint64_t val) {
  if (!is_alligned(size, addr)) {
    invoke_runtime_error("address not aligned");
    return 0;
  }

  double cost;
  int temp_old = this->temp.on_access(addr);
  if (is_stack(size, addr)) {
    store_stack(size, addr, val);
    cost = stack_cost(temp_old);
    _store_stack += Cost::STACK;
    _temp += Cost::PER_TEMP * temp_old;
  } else if (is_heap(size, addr)) {
    store_heap(size, addr, val);
    cost = heap_cost(temp_old);
    _store_heap += Cost::HEAP;
    _temp += Cost::PER_TEMP * temp_old;
  } else {
    invoke_runtime_error("accessing address between 102400 and 204800");
    return 0;
  }

  return cost;
}

double Memory::exec_vload(VSize size, uint64_t addr, uint64_t* results, const bool* mask) {
  MSize msize = MSize8;
  double max_cost = 0.0;

  if (!is_alligned(msize, addr)) {
    invoke_runtime_error("address not aligned");
    return 0;
  }

  for (int i = 0; i < vsize_of(size); i++) {
    if (!mask[i]) continue;

    uint64_t addr_curr = addr + msize_of(msize) * i;
    int temperature = temp.on_access(addr_curr);
    double cost = 0;

    if (is_stack(msize, addr_curr)) {
      results[i] = load_stack(msize, addr_curr);
      cost = vstack_cost(temperature);
    } else if (is_heap(msize, addr_curr)) {
      results[i] = load_heap(msize, addr_curr);
      cost = vheap_cost(temperature);
    } else {
      invoke_runtime_error("accessing address between 102400 and 204800");
      return 0;
    }

    if (cost > max_cost)
      max_cost = cost;
  }

  if (is_stack(msize, addr)) {
    _vload_stack += Cost::VSTACK;
    _vtemp += max_cost - Cost::VSTACK;
  }
  else {
    _vload_heap += Cost::VHEAP;
    _vtemp += max_cost - Cost::VHEAP;
  }
  return max_cost;
}

double Memory::exec_vstore(VSize size, uint64_t addr, uint64_t* vals, const bool* mask) {
  MSize msize = MSize8;
  double max_cost = 0.0;

  if (!is_alligned(msize, addr)) {
    invoke_runtime_error("address not aligned");
    return 0;
  }

  for (int i = 0; i < vsize_of(size); i++) {
    if (!mask[i]) continue;

    uint64_t addr_curr = addr + msize_of(msize) * i;
    int temperature = temp.on_access(addr_curr);
    double cost = 0;

    if (is_stack(msize, addr_curr)) {
      store_stack(msize, addr_curr, vals[i]);
      cost = vstack_cost(temperature);
    } else if (is_heap(msize, addr_curr)) {
      store_heap(msize, addr_curr, vals[i]);
      cost = vheap_cost(temperature);
    } else {
      invoke_runtime_error("accessing address between 102400 and 204800");
      return 0;
    }

    if (cost > max_cost)
      max_cost = cost;
  }

  if (is_stack(msize, addr)) {
    _vstore_stack += Cost::VSTACK;
    _vtemp += max_cost - Cost::VSTACK;
  }
  else {
    _vstore_heap += Cost::VHEAP;
    _vtemp += max_cost - Cost::VHEAP;
  }
  return max_cost;
}

double Memory::exec_malloc(uint64_t size, uint64_t& result) {
  if (size == 0)
    invoke_runtime_error("allocation size should not be 0");
  if (size % 8 != 0)
    invoke_runtime_error("allocation size should be multiple of 8");

  for (auto it = freed.begin(); it != freed.end(); it++) {
    if (it->second - it->first < size)
      continue;

    block_t block = *it;
    freed.erase(block);
    freed.insert(block_t(block.first + size, block.second));
    auto ptr = (uint8_t*)calloc(size, sizeof(uint8_t));

    if (ptr == nullptr) {
      invoke_runtime_error("out-of-memory");
      return 0;
    }

    alloced.insert(alloc_t(block_t(block.first, block.first + size), ptr));
    result = block.first;
    alloced_size += size;
    if (max_alloced_size < alloced_size)
      max_alloced_size = alloced_size;
    _malloc += Cost::MALLOC;
    return Cost::MALLOC;
  }

  invoke_runtime_error("out-of-memory");
  return 0;
}

double Memory::exec_free(uint64_t addr) {
  auto alloc_lb = alloced.lower_bound(alloc_t(block_t(addr, 0), nullptr));

  if (alloc_lb == alloced.end() || alloc_lb->first.first != addr) {
    invoke_runtime_error("freeing non-allocated address");
    return 0;
  }

  block_t block = alloc_lb->first;
  uint8_t* ptr = alloc_lb->second;
  uint64_t size = block.second - block.first;
  alloced.erase(alloc_lb);
  free(ptr);

#ifdef USE_MEMORY_TEMP
  for (uint64_t addr_curr = addr; addr_curr < addr + size; addr_curr += 8)
    temp.remove(addr_curr);
#endif

  auto next = freed.lower_bound(block);

  if (next != freed.begin()) {
    auto prev = next;
    prev--;

    if (prev->second == block.first) {
      block = block_t(prev->first, block.second);
      freed.erase(prev);
    }
  }

  if (next != freed.end()) {
    if (next->first == block.second) {
      block = block_t(block.first, next->second);
      freed.erase(next);
    }
  }

  freed.insert(block);
  alloced_size -= size;
  _free += Cost::FREE;
  return Cost::FREE;
}

uint64_t Memory::get_alloced_size() const { return alloced_size; }

uint64_t Memory::get_max_alloced_size() const { return max_alloced_size; }

void Memory::advance_time() {
#ifdef USE_MEMORY_TEMP
  temp.advance_tick();
#endif // USE_MEMORY_TEMP
}

void Memory::exec_cool(uint64_t addr) {
  if (!is_alligned(MSize8, addr))
    invoke_runtime_error("an address being cooled should be multiple of 8");
  temp.freeze(addr);
}