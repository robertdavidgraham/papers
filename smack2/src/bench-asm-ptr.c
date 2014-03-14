#include "bench.h"
#include "pixie-rdtsc.h"

void
bench_asm_ptr(void)
{
    void *v;
    unsigned long long start, stop;
    unsigned i;
    
    /*
     * Create a pointer that points to itself. Thus, as we follow this
     * pointer, we don't actually change it.
     */
    v = (void*)&v;
    
    i = ITERATIONS;
    start = rdtsc();
    __asm__ __volatile 
    (
     "again:\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "mov (%1), %1\n"
     "dec %0\n"
     "jnz again\n"
     
     :
     : "r" (i), "r" (v)
     
     );
    stop = rdtsc();
    
    {
        unsigned long long elapsed = stop - start;
        double clocks = elapsed/30.0/ITERATIONS;
        printf("asm-ptr = %5.3f-clocks\n", clocks);
    }
}
