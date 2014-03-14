#include "bench.h"
#include "pixie-rdtsc.h"

void
bench_c_ptr(void)
{
    void *v;
    unsigned long long start, stop;
    unsigned i;
    
    /*
     * Create a pointer that points to itself. Thus, as we follow this
     * pointer, we don't actually change it.
     */
    v = (void*)&v;
    
    start = rdtsc();
    for (i=0; i<ITERATIONS; i++) {
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
    stop = rdtsc();
    
    {
        unsigned long long elapsed = stop - start;
        double clocks = elapsed/20.0/ITERATIONS;
        printf("c-ptr = %5.3f-clocks\n", clocks);
    }
}
