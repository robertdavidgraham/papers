#ifndef smack2_bench_h
#define smack2_bench_h
#include <stdio.h>

#define ITERATIONS 10000000

void bench_c_ptr(void);
void bench_c_idx(unsigned idx);
void bench_asm_ptr(void);
void bench_asm_ptr2(void);
void bench_asm_idx(void);

#endif
