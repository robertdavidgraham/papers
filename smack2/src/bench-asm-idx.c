#include "bench.h"
#include "pixie-rdtsc.h"
#include <string.h>

void
bench_asm_idx(void)
{
    unsigned table[10001];
    size_t index = 0;
    unsigned long long start, stop;
    unsigned i;
    unsigned *p = &table[0];
    
    memset(table,0,sizeof(table));
    for (i=0; i<10000; i++)
        table[i+1] = i;
    table[0] = 9999;
    
    
    i = ITERATIONS;
    start = __rdtsc();
#if defined(__GNUC__) && (defined(__amd64__) || defined(__x86_64__))
    __asm__ __volatile 
    (
     "again:\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "movslq (%[table],%[index],4), %[index]\n"
     "dec %[i]\n"
     "jnz again\n"
     : [index] "=r" (index)
     : [i]  "r" (i), [table] "r" (p), "[index]" (index)
     : "%rax"
     );
#elif defined (__GNUC__) && defined(__i386__)
    __asm__ __volatile 
    (
     "again:\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "mov (%[table],%[index],4), %[index]\n"
     "dec %[i]\n"
     "jnz again\n"
     : [index] "=r" (index)
     : [i]  "r" (i), [table] "r" (p), "[index]" (index)
     : "%rax"
     );
#elif defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        push eax
        push ebx
        push esi

        mov eax, i
        mov ebx, p
        mov esi, index
again:

        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]
        mov esi, dword ptr[ebx + esi*4]

        dec eax
        jg again

        pop esi
        pop ebx
        pop eax
    };

#else
//#error unknown processor
#endif
    stop = __rdtsc();
    
    {
        unsigned long long elapsed = stop - start;
        double clocks = elapsed/20.0/ITERATIONS;
        printf("asm-idx = %5.3f-clocks\n", clocks);
    }
}
