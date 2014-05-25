#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <pthread.h>
#include <unistd.h>

#define BUF_SIZE (1024*1024*1024)

struct memtest {
    size_t loop_count;
    char *buf;
};
volatile unsigned threads_running = 0;


/**
 * A fast random number generator. This only has to be random enough so that
 * memory accesses are uncacheable. However, the built-in rand() function
 * does long division, which has very large latency, which skews the results.
 * Therefore, I pulled this off the net as an alternative random number
 * generator that takes fewer clock cycles.
 */
typedef struct ranctx { 
    unsigned a, b, c, d;
} ranctx;
unsigned ranval(ranctx *x) 
{
#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
    unsigned e = x->a - rot(x->b, 27);
    x->a = x->b ^ rot(x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}
void raninit(ranctx *x, unsigned seed) 
{
    unsigned i;
    x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
    for (i=0; i<20; ++i) {
        (void)ranval(x);
    }
}



/******************************************************************************
 ******************************************************************************/
#if defined(__MACH__) || defined(__FreeBSD__) /* works for Apple */
#include <unistd.h>
#include <mach/mach_time.h>
uint64_t
pixie_gettime(void)
{
    return mach_absolute_time()/1000;
}
void pixie_usleep(uint64_t microseconds)
{
    struct timespec t;
    t.tv_nsec = microseconds * 1000;
    if (microseconds > 1000000)
        t.tv_sec = microseconds/1000000;
    else
        t.tv_sec = 0;
    nanosleep(&t, 0);
}
#elif defined(CLOCK_MONOTONIC)
uint64_t
pixie_gettime(void)
{
    int x;
    struct timespec tv;
    
#ifdef CLOCK_MONOTONIC_RAW
    x = clock_gettime(CLOCK_MONOTONIC_RAW, &tv);
#else
    x = clock_gettime(CLOCK_MONOTONIC, &tv);
#endif
    if (x != 0) {
        printf("clock_gettime() err %d\n", errno);
    }
    
    return tv.tv_sec * 1000000 + tv.tv_nsec/1000;
}
#endif


/******************************************************************************
 ******************************************************************************/
void test_thread(struct memtest *memtest)
{
    size_t i;
    unsigned result = 0;
    size_t offset = 0;
    uint64_t start, stop;
    ranctx x;
    char *buf = memtest->buf;
    
    raninit(&x, pixie_gettime());
    
    
    
    start = pixie_gettime();
    for (i=0; i<memtest->loop_count; i++) {
        result += buf[offset];
        offset += ranval(&x);
        offset &= (1024*1024*1024 - 1);
    }
    stop = pixie_gettime();
    result += offset + i;
    
    {
        double rate = (i*1.0)/(stop-start);
        
        printf("rate = %5.3f (%llu)\n", rate, (unsigned long long)result);
    }
    
    __sync_fetch_and_sub(&threads_running, 1);
}


/******************************************************************************
 ******************************************************************************/
int main(int argc, char *argv[])
{
    unsigned i;
    unsigned threads_desired = 4;
    uint64_t start, stop;
    struct memtest memtest;

    fprintf(stderr, "running %u threads\n", threads_desired);

    memset(&memtest, 0, sizeof(memtest));
    memtest.loop_count = 1024*1024*64;
    
    /* Allocate a buffer, we then set it to zero to make sure that the
     * operating system has paged the entire thing into memory, so that page
     * faults don't skew the results */
    memtest.buf = malloc(BUF_SIZE);
    memset(memtest.buf, 0, BUF_SIZE);


    /* 
     * Launch worker threads, one per core. Then, wait for thread to end.
     * This is done poorly, such that some threads end a lot sooner than
     * others, but really, this only effects the results by aboug 1%, so
     * so I don't care.
     */
    start = pixie_gettime();
    for (i=0; i<threads_desired; i++) {
        pthread_t thread_id;
        __sync_fetch_and_add(&threads_running, 1);
        pthread_create(&thread_id, 0, test_thread, &memtest);
    }
    while (threads_running)
        pixie_usleep(1000);
    stop = pixie_gettime();
    
    /*
     * Print the results
     */
    {
        double rate = (memtest.loop_count*threads_desired*1.0)/(stop-start);
        printf("rate = %5.3f\n", rate);
    }
    
    return 0;
}


