#ifndef PIXIE_RDTSC_H
#define PIXIE_RDTSC_H

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
static __inline__ unsigned long long rdtsc(void)
{
    unsigned long hi = 0, lo = 0;
    __asm__ __volatile__ ("lfence\n\trdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#endif


#endif
