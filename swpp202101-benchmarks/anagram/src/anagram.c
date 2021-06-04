#include <inttypes.h>
#include <stdlib.h>

uint64_t read();
void write(uint64_t val);

int count(uint8_t val, uint64_t len, uint8_t* str) {
  int count = 0;
  for (int i = 0; i < len; i++)
    if (val == str[i])
      count++;
  return count;
}

int anagram(uint64_t len, uint8_t* str1, uint8_t* str2) {
  for (uint8_t val = 0; val < UINT8_MAX; val++) {
    int count1 = count(val, len, str1);
    int count2 = count(val, len, str2);
    if (count1 != count2)
      return 0;
  }
  return 1;
}


int main() {
  uint64_t len = read();

  if (len == 0)
    return 0;

  uint8_t* str1 = (uint8_t*)malloc(((len + 7) / 8) * 8 * sizeof(uint8_t));
  uint8_t* str2 = (uint8_t*)malloc(((len + 7) / 8) * 8 * sizeof(uint8_t));

  for (int i = 0; i < len; i++)
    str1[i] = (uint8_t)(read() % 256);
  for (int i = 0; i < len; i++)
    str2[i] = (uint8_t)(read() % 256);

  int ret = anagram(len, str1, str2);
  write(ret);

  return 0;
}
