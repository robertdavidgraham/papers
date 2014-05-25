#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#include "pixie-rdtsc.h"
#include "pixie-cpuid.h"
#include "pixie-timer.h"
#include "bench.h"
#include "smack.h"

#if defined(WIN32)
#include <direct.h>
#include <Windows.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#ifdef __linux__
#define _USE_GNU
#define __USE_GNU
#include <sched.h>
#endif

/******************************************************************************
 * simply get command-line parameter
 ******************************************************************************/
static const char *
get_parm(const char *name, int argc, char *argv[])
{
    int i;
    for (i=1; i<argc; i++) {
        if (strcmp(name, argv[i]) == 0) {
            if (i+1 >= argc)
                return "";
            return argv[i+1];
        }
    }
    return 0;
}

/******************************************************************************
 ******************************************************************************/
static void
do_haystack(const struct SMACK *smack,
            const char *buffer, size_t sizeof_buffer,
            unsigned long long iterations)
{
    unsigned long long start, stop;
    unsigned long long cycle1, cycle2;
    unsigned i;
    unsigned found_count = 0;
    
    start = pixie_nanotime();
    cycle1 = __rdtsc();

    for (i=0; i<iterations; i++) {
        unsigned state = 0;
        unsigned offset = 0;
        
        while (offset < sizeof_buffer) {
            size_t x;
        
            x = smack_search_next(smack, 
                                  &state,
                                  buffer,
                                  &offset,
                                  sizeof_buffer);
            if (x != SMACK_NOT_FOUND)
                found_count++;
        }
    }
    
    cycle2 = __rdtsc();
    stop = pixie_nanotime();
    
    {
        double elapsed = ((double)(stop - start))/(1000000000.0);
        double rate = (sizeof_buffer*iterations*8ULL)/elapsed;
        double cycles = (sizeof_buffer*iterations*1.0)/(1.0*(cycle2-cycle1));
        
        rate /= 1000000.0;
        
        printf("Found count = %u\n", (unsigned)(found_count/iterations));
        printf("Search speed = %5.3f-gbps (%3.2f-Hz/byte)\n", rate/1000.0, (1.0/cycles));
        printf("CPU speed = %5.3f-GHz\n", ((cycle2-cycle1)*1.0/elapsed)/1000000000.0);        
    }

}

/******************************************************************************
 ******************************************************************************/
int
main(int argc, char *argv[])
{
    printf("---- SMACK/2 - benchmark program ----\n");

    {
        char dir[512];
        char *p = getcwd(dir, sizeof(dir));
	if (p == NULL)
		perror("getcwd");
	else
        	printf("directory = %s\n", p);
    }

#if WIN32
    {
        SetProcessAffinityMask(NULL, 1);
        SetPriorityClass(NULL,  REALTIME_PRIORITY_CLASS);
    }
#endif
#ifdef __linux__

    {
        cpu_set_t mask;
        int status;

        CPU_ZERO(&mask);
        CPU_SET(1, &mask);
        status = sched_setaffinity(0, sizeof(mask), &mask);
        if (status != 0)
        {
            perror("sched_setaffinity");
        }
    }

#endif

    /*
     * Print cpu info
     */
    {
        char vendor[16];
        char brand[64];
        struct CpuInfo info;
        
        pixie_cpu_vendor(vendor);
        pixie_cpu_brand(brand);
        pixie_cpu_info(&info);

        printf("CPU width = %u-bits\n", (unsigned)sizeof(void*)*8);
        printf("CPU vendor = \"%s\"\n", vendor);
        printf("CPU brand = \"%s\"\n", brand);
        printf("CPU codename = \"%s\" (0x%X" "x)\n", info.codename, info.codenumber);
        printf("CPU info = type(%u) family(%u) model(%u) stepping(%u)\n",
               info.type, info.family, info.model, info.stepping);
        
        printf("\n");
    }

    /*
     * First, do a unit-test. If the unit-test fails, then it's pointless
     * continueing with the benchmarks
     */
    if (smack_selftest() != 0) {
        printf("selftest failed\n");
        return 1;
    }
    
    /*
     * Print help if user is confused
     */
    if (0 && ( argc <= 1 
        || get_parm("--help", argc, argv) 
        || get_parm("-h", argc, argv)
        || get_parm("-?", argc, argv))) {
        printf("usage:\n"
               " smack2 --benchmark\n"
               " smack2 --haystack kingjames.txt --needle pharoah\n"
               " smack2 --haystack kingjames.txt --needles patterns.txt\n"
               );
        return 1;
    }
    if (get_parm("--haystack", argc, argv)) {
        if (!get_parm("--needles", argc, argv) 
            && !get_parm("--needle", argc, argv)) {
            printf("FAIL: need to specify patterns to search for\n");
            printf(" hint: use \"--needles <file>\" to read patterns from file\n");
            printf(" hint: use \"--needle <pattern\" to specify one or more patterns\n");
            return 1;
        }
    }
        
    
    /*
     * If doing simple benchmarks, do them first
     */
    if (get_parm("--benchmark", argc, argv) || argc <= 1) {
        printf("--- low-level benchmarks ---\n");
        bench_c_ptr();
        bench_c_idx(0);
        bench_asm_ptr();
        //bench_asm_ptr2();
        bench_asm_idx();
        printf("\n");
        smack_benchmark();
    }
    if (!get_parm("--haystack", argc, argv))
        return 0;
    
    /*
     * Look in file for patterns
     */
    {
        const char *filename = get_parm("--haystack", argc, argv);
        char *buf;
        size_t sizeof_buf;
        struct stat s;
        int x;
        struct SMACK *smack;
        int i;
        
        fprintf(stderr, "haystack file = %s\n", filename);
        
        /*
         * Find file size
         */
        x = stat(filename, &s);
        if (x != 0) {
            perror(filename);
            return 1;
        }
        sizeof_buf = s.st_size;
        
        /*
         * Allocate memory
         */
        buf = (char*)malloc(sizeof_buf);
        if (buf == 0) {
            fprintf(stderr, "Could not allocate %u bytes for buffer\n",
                    (unsigned)sizeof_buf);
            return 1;
        }
        
        /*
         * Read in the file
         */
        {
            FILE *fp;
            size_t bytes_read;
            fp = fopen(filename, "rb");
            if (fp == NULL) {
                perror(filename);
                return 1;
            }
            bytes_read = fread(buf, 1, sizeof_buf, fp);
            if (bytes_read == 0) {
                perror(filename);
                return 1;
            }
            if (bytes_read < sizeof_buf) {
                fprintf(stderr, "ERROR: could not read entire file\n");
                fprintf(stderr, "wanted %u bytes, read %u bytes\n",
                        (unsigned)sizeof_buf, 
                        (unsigned)bytes_read);
                sizeof_buf = bytes_read;
            }
            fclose(fp);
            
            fprintf(stderr, "file-size = %u\n", (unsigned)sizeof_buf);
        }
        
        /*
         * Build the smack object
         */
        smack = smack_create("haystack", SMACK_CASE_INSENSITIVE);
        if (smack == 0) {
            fprintf(stderr, "FAIL: couldn't create test object\n");
            return 1;
        }
        
        /*
         * Add patterns
         */
        for (i=1; i<argc; i++) {
            if (strcmp(argv[i], "--needle") == 0) {
                const char *pattern = argv[i+1];
                if (i+1 >= argc) {
                    fprintf(stderr, "FAIL: expected value to parm\n");
                    return 1;
                }
                smack_add_pattern(smack, pattern, strlen(pattern), 1, 0);
            }
            if (strcmp(argv[i], "--needles") == 0) {
                const char *patterns = argv[i+1];
                FILE *fp;
                char line[80];
                if (i+1 >= argc) {
                    fprintf(stderr, "FAIL: expected value to parm\n");
                    return 1;
                }
                fp = fopen(patterns, "rt");
                if (fp == NULL) {
                    perror(patterns);
                    return 1;
                }
                
                while (fgets(line, sizeof(line), fp)) {
                    /* strip whitespace */
                    while (line[0] && isspace(line[0]&0xFF))
                        memmove(line, line+1, strlen(line));
                    while (line[0] && isspace(line[strlen(line)-1]&0xFF))
                        line[strlen(line)-1] = '\0';
                    if (line[0] == '\0' || ispunct(line[0]&0xFF))
                        continue;
                    smack_add_pattern(smack, line, strlen(line), 1, 0);
                }
            }
        }
        if (smack_count_patterns(smack) == 0) {
            fprintf(stderr, "FAIL: no patterns found\n");
            return 1;
        } else {
            fprintf(stderr, "needles = %u patterns\n", smack_count_patterns(smack));
        }
        
        smack_compile(smack);
        
        /*
         * Do the benchmark
         */
        do_haystack(smack,
                    buf, sizeof_buf,
                    10);

    }
    
    
    
    return 0;
}

