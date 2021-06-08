#include <stdint.h>
#include <stdlib.h>

uint64_t read();
void write(uint64_t val);

extern uint64_t global[256];

void func(uint64_t *temp1, uint64_t *temp2)
{
    uint64_t local[256];
    uint64_t r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28;
    uint64_t r29, r30, r31, r32;

    r1 = read(), r2 = read(), r3 = read(), r4 = read(), r5 = read();
    r6 = read(), r7 = read(), r8 = read(), r9 = read(), r10 = read();
    r11 = read(), r12 = read(), r13 = read(), r14 = read(), r15 = read();
    r16 = read(), r17 = read(), r18 = read(), r19 = read(), r20 = read();
    r21 = read(), r22 = read(), r23 = read(), r24 = read(), r25 = read();
    r26 = read(), r27 = read(), r28 = read(), r29 = read(), r30 = read();
    r31 = read(), r32 = read();

    write(r1), write(r2), write(r3), write(r4), write(r5), write(r6);
    write(r7), write(r8), write(r9), write(r10), write(r11), write(r12);
    write(r13), write(r14), write(r15), write(r16), write(r17), write(r18);
    write(r19), write(r20), write(r21), write(r22), write(r23), write(r24);
    write(r25), write(r26), write(r27), write(r28), write(r29), write(r30);
    write(r31), write(r32);
}

int main()
{
    uint64_t local[256];
    uint64_t r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t r16, r17, r18, r19, r20, r21, r22, r23, r24, r25, r26, r27, r28;
    uint64_t r29, r30, r31, r32;

    uint64_t *temp = malloc(sizeof(uint64_t) * 256);

    r1 = read(), r2 = read(), r3 = read(), r4 = read(), r5 = read();
    r6 = read(), r7 = read(), r8 = read(), r9 = read(), r10 = read();
    r11 = read(), r12 = read(), r13 = read(), r14 = read(), r15 = read();
    r16 = read(), r17 = read(), r18 = read(), r19 = read(), r20 = read();
    r21 = read(), r22 = read(), r23 = read(), r24 = read(), r25 = read();
    r26 = read(), r27 = read(), r28 = read(), r29 = read(), r30 = read();
    r31 = read(), r32 = read();

    func(global, local);

    write(r1), write(r2), write(r3), write(r4), write(r5), write(r6);
    write(r7), write(r8), write(r9), write(r10), write(r11), write(r12);
    write(r13), write(r14), write(r15), write(r16), write(r17), write(r18);
    write(r19), write(r20), write(r21), write(r22), write(r23), write(r24);
    write(r25), write(r26), write(r27), write(r28), write(r29), write(r30);
    write(r31), write(r32);
    
    return 0;
}
