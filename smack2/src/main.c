#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#include "pixie-rdtsc.h"
#include "bench.h"
#include "smack.h"


/******************************************************************************
 ******************************************************************************/
#ifndef NOBENCHMARK
#include "pixie-timer.h"
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__)
static __inline__ unsigned long long __rdtsc(void)
{
#if defined(i386) || defined(__i386__) || defined(__x86_64__) || defined(__amd64)
    unsigned long hi = 0, lo = 0;
    __asm__ __volatile__ ("lfence\n\trdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
#else
    return 0;
#endif
}

static __inline__ void __cpu_vendor(char vendor[16])
{
#if defined(i386) || defined(__i386__) || defined(__x86_64__) || defined(__amd64)
    asm volatile ( 
                  "mov $0, %%eax\n"
                  "cpuid\n"
                  "mov %%ebx, (%[vendor])\n"
                  "mov %%edx, 4(%[vendor])\n"
                  "mov %%ecx, 8(%[vendor])\n"
                  "movb $0, 12(%[vendor])\n" 
                  :
                  : [vendor] "r"(vendor) : "eax", "ebx", "ecx", "edx" );
#else
    strcpy(vendor, "unknown");
#endif
}


struct CpuInfo {
    unsigned stepping;
    unsigned model;
    unsigned family;
    unsigned type;
    const char *codename;
    unsigned codenumber;
};

static __inline__ void __cpu_info(struct CpuInfo *info)
{
    unsigned x;
#if defined(i386) || defined(__i386__) || defined(__x86_64__) || defined(__amd64)
    asm volatile ( 
                  "mov $1, %%eax\n"
                  "cpuid\n"
                  : "=a" (x)
                  );
    //printf("0x%08x\n", x);
    /*
     3:0 – Stepping
     7:4 – Model
     11:8 – Family
     13:12 – Processor Type
     19:16 – Extended Model
     27:20 – Extended Family
     */
    info->stepping = (x>>0) & 0xF;
    info->model = (x>>4) & 0xF;
    info->family = (x>>8) & 0xF;
    info->type = (x>>12) & 0x3;
    info->model |= (x>>12) & 0xF0;
    info->family |= (x>>20) & 0xFF0;
    info->codenumber = x>>4;
    switch (x>>4) {
        case 0x306A: info->codename = "Ivy Bridge"; break;
        case 0x206A: info->codename = "Sandy Bridge"; break;
        case 0x206D: info->codename = "Sandy Bridge-E"; break;
        case 0x2065: info->codename = "Westmere"; break;
        case 0x206C: info->codename = "Westmere-EP"; break;
        case 0x206F: info->codename = "Westmere-EX"; break;
        case 0x106E: info->codename = "Nehalem"; break;
        case 0x106A: info->codename = "Nehalem-EP"; break;
        case 0x206E: info->codename = "Nehalem-EX"; break;
        case 0x1067: info->codename = "Penryn"; break;
        case 0x106D: info->codename = "Penryn-E"; break;
        case 0x006F: info->codename = "Merom"; break;
        case 0x1066: info->codename = "Merom"; break;
        case 0x0066: info->codename = "Presler"; break;
        case 0x0063: info->codename = "Prescott"; break;
        case 0x0064: info->codename = "Prescott"; break;
        case 0x006D: info->codename = "Dothan"; break;
        
        case 0x0366: info->codename = "Cedarview"; break;
        case 0x0266: info->codename = "Lincroft"; break;
        case 0x016C: info->codename = "Pineview"; break;
        default: info->codename = "Unknown"; break;
    }
#else
    strcpy(vendor, "unknown");
#endif
}

static __inline__ void __cpu_model(char model[64])
{
#if defined(i386) || defined(__i386__) || defined(__x86_64__) || defined(__amd64)
    asm volatile ( 
                  "mov $0x80000002, %%eax\n"
                  "cpuid\n"
                  "mov %%eax, (%[model])\n"
                  "mov %%ebx, 4(%[model])\n"
                  "mov %%ecx, 8(%[model])\n"
                  "mov %%edx, 12(%[model])\n"
                  "mov $0x80000003, %%eax\n"
                  "cpuid\n"
                  "mov %%eax, 16(%[model])\n"
                  "mov %%ebx, 20(%[model])\n"
                  "mov %%ecx, 24(%[model])\n"
                  "mov %%edx, 28(%[model])\n"
                  "mov $0x80000004, %%eax\n"
                  "cpuid\n"
                  "mov %%eax, 32(%[model])\n"
                  "mov %%ebx, 46(%[model])\n"
                  "mov %%ecx, 40(%[model])\n"
                  "mov %%edx, 44(%[model])\n"
                  "movb $0, 48(%[model])\n" 
                  :
                  : [model] "r"(model) : "eax", "ebx", "ecx", "edx" );
    while (model[0] && isspace(model[0]&0xFF))
        memmove(model, model+1, strlen(model));
#else
    strcpy(vendor, "unknown");
#endif
}

#endif
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
        char vendor[16];
        char model[64];
        struct CpuInfo info;
        
        __cpu_vendor(vendor);
        __cpu_model(model);
        __cpu_info(&info);
        
        rate /= 1000000.0;
        
        printf("Found count = %u\n", (unsigned)(found_count/iterations));
        printf("Search speed = %5.3f-gbps (%3.2f-Hz/byte)\n", rate/1000.0, (1.0/cycles));
        printf("CPU width = %u-bits\n", (unsigned)sizeof(void*)*8);
        printf("CPU speed = %5.3f-GHz\n", ((cycle2-cycle1)*1.0/elapsed)/1000000000.0);
        printf("CPU vendor = \"%s\"\n", vendor);
        printf("CPU brand = \"%s\"\n", model);
        printf("CPU codename = \"%s\" (0x%X" "x)\n", info.codename, info.codenumber);
        printf("CPU info = type(%u) family(%u) model(%u) stepping(%u)\n",
               info.type, info.family, info.model, info.stepping);
        
        printf("\n");
        
    }

}

/******************************************************************************
 ******************************************************************************/
int
main(int argc, char *argv[])
{
    /*{
        char dir[512];
        getcwd(dir, sizeof(dir));
        printf("%s\n", dir);
    }*/
    
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
        bench_asm_ptr2();
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
            fprintf(stderr, "Could not allocate %llu bytes for buffer\n",
                    (unsigned long long)sizeof_buf);
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
                fprintf(stderr, "wanted %llu bytes, read %llu bytes\n",
                        (unsigned long long)sizeof_buf, 
                        (unsigned long long)bytes_read);
                sizeof_buf = bytes_read;
            }
            fclose(fp);
            
            fprintf(stderr, "file-size = %llu\n", (unsigned long long)sizeof_buf);
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

