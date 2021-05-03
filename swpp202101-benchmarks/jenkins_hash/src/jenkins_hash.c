// https://en.wikipedia.org/wiki/Jenkins_hash_function
#include <inttypes.h>
#include <stdlib.h>

uint64_t read();
void write(uint64_t val);

void *malloc_upto_8(uint64_t x) {
  return malloc((x + 7) / 8 * 8);
}

uint32_t jenkins_one_at_a_time_hash(uint32_t value, const uint8_t* key, size_t length) {
  size_t i = 0;
  uint32_t hash = value;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}


int main() {
    uint64_t n = read();
    if (n == 0) {
      write(0);
      return 0;
    }
    uint8_t *arr = (uint8_t *)malloc_upto_8(n);
    for (int i = 0; i < n; ++i)
        arr[i] = (uint8_t)read();

    uint32_t value = 0;
    for (int i = 0; i < 10; ++i)
      value = jenkins_one_at_a_time_hash(value, arr, n);

    write(value);

    free(arr);

    return 0;
}