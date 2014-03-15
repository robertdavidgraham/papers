#include "bench.h"
#include "pixie-rdtsc.h"

void
bench_asm_ptr2(void)
{
    void *v;
    void *v2;
    unsigned long long start, stop;
    unsigned i;
    
    /*
     * Create a pointer that points to itself. Thus, as we follow this
     * pointer, we don't actually change it.
     */
    v = (void*)&v2;
    v2 = (void*)&v;
    
    i = ITERATIONS;
    start = __rdtsc();
#if defined(__GNUC__) && (defined(__i386__) || defined(__amd64__))
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
     "dec %0\n"
     "jnz again\n"
     
     :
     : "r" (i), "r" (v)
     
     );
#else
    while (i--) {
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
        v = *(void**)v;
    }
#endif
    stop = __rdtsc();
    
    {
        unsigned long long elapsed = stop - start;
        double clocks = elapsed/20.0/ITERATIONS;
        printf("asm-ptr2 = %5.3f-clocks\n", clocks);
    }
}
