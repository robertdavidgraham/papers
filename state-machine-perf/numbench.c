#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/time.h>
#include <netinet/in.h>

#if defined(__MACH__)
#include <unistd.h>
#include <mach/mach_time.h>
#endif

#if defined(__GNUC__)
#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

#elif defined(__x86_64__)

static __inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

#endif
#endif









#define BUF_LEN 1024*4
#define REPEAT_COUNT 1024*1024

/**
 * Custom randomization function so that the results are reproducible
 * regardless of compiler/library version.
 */
static unsigned
r_rand(unsigned *seed)
{
    static const unsigned a = 214013;
    static const unsigned c = 2531011;
    
    *seed = (*seed) * a + c;
    return (*seed)>>16 & 0x7fff;
}



int main(int argc, char *argv[])
{
    unsigned char *px;
    unsigned i;
    unsigned long long start, stop;
    unsigned long long block_clocks = 0;
    unsigned long long stream_clocks = 0;
    unsigned block_result = 0;
    unsigned stream_result = 0;
    
    /*
     * Create a buffer with random contents
     */
    {
        unsigned seed = 0;
        px = (unsigned char *)malloc(BUF_LEN);
        
        for (i=0; i<BUF_LEN; i++)
            px[i] = (unsigned char)r_rand(&seed);
    }
    
    
    
    
    /*
     * Do both tests back and forth in a repeated loop, in order to avoid
     * effects of CPU ramping up
     */
    for (i=0; i<REPEAT_COUNT; i++) {
        unsigned j;
        
        /* block integer extraction */
        start = rdtsc();    
        for (j=0; j<BUF_LEN; j += 4) {
            block_result += ntohl(*(unsigned *)(px+j));
        }
        stop = rdtsc();
        block_clocks += (stop - start);
        
        /* stream integer extraction */
        start = rdtsc();    
        for (j=0; j<BUF_LEN; j += 4) {
            stream_result += px[j]<<24 | px[j+1]<<16 | px[j+2]<<8 | px[j+3];
        }
        stop = rdtsc();
        stream_clocks += (stop - start);
    }
    
    /*
     * Print results
     */
    {
        double block_time = (double)block_clocks / (1.0*REPEAT_COUNT*(BUF_LEN/4));
        double stream_time = (double)stream_clocks / (1.0*REPEAT_COUNT*(BUF_LEN/4));
        
        printf("block algo  = %5.3f clocks/integer\n", block_time);
        printf("stream algo = %5.3f clocks/integer\n", stream_time);
        
        printf("block result  = 0x%08x\n", block_result);
        printf("stream result = 0x%08x\n", stream_result);
    }    
    return 0;
}
