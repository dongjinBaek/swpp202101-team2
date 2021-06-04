#include <inttypes.h>
#include <stdlib.h>

uint64_t read();
void write(uint64_t val);

extern uint16_t* graph;
extern uint64_t cycle_pos;
extern uint64_t cycle_len;

void *malloc_upto_8(uint64_t x) {
  return malloc((x + 7) / 8 * 8);
}

uint16_t* advance(uint16_t* node) {
    size_t idx = *node;
    return &graph[idx];
}

// find position and length of cycle
// using Floyd's tortoise-hare algorithm
// https://en.wikipedia.org/wiki/Cycle_detection
void floyd() {
    uint16_t* tortoise = advance(graph);
    uint16_t* hare = advance(advance(graph));
    while (tortoise != hare) {
        tortoise = advance(tortoise);
        hare = advance(advance(hare));
    }

    cycle_pos = 0;
    tortoise = graph;
    while (tortoise != hare) {
        tortoise = advance(tortoise);
        hare = advance(hare);
        cycle_pos += 1;
    }

    cycle_len = 1;
    hare = advance(tortoise);
    while (tortoise != hare) {
        hare = advance(hare);
        cycle_len += 1;
    }
}

int main() {
    uint64_t nodes = read();
    graph = (uint16_t*)malloc_upto_8(nodes * 2);

    for (size_t i = 0; i < nodes; i++) {
        graph[i] = (uint16_t)read();
    }

    floyd();
    write(cycle_pos);
    write(cycle_len);

    return 0;
}