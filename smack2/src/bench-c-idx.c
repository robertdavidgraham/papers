#include "bench.h"
#include "pixie-rdtsc.h"

void
bench_c_idx(unsigned row)
{
    void *v;
    unsigned long long start, stop;
    unsigned i;
    
    /*
     * Create a pointer that points to itself. Thus, as we follow this
     * pointer, we don't actually change it.
     */
    v = (void*)&v;
    
    start = __rdtsc();
    for (i=0; i<ITERATIONS; i++) {
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
		v = ((void**)v)[row];
    }
    stop = __rdtsc();
    
    {
        unsigned long long elapsed = stop - start;
        double clocks = elapsed/20.0/ITERATIONS;
        printf("c-idx = %5.3f-clocks\n", clocks);
    }
}
