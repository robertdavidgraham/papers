#include "pixie-cpuid.h"

#if defined(__GNUC__)
void __cpuid(int results[4], int type)
{
    __asm volatile ( 
                  "cpuid"
                  : "=a" (results[0]), "=b" (results[1]), "=c" (results[2]), "=d" (results[3])
                  : "a"(type)
                  
    );
}
#else
#include <intrin.h>
#endif


void
pixie_cpu_vendor(char vendor[16])
{
    int foo[4] = {0,0,0,0};

    __cpuid(foo, 0);

    *(int*)(vendor+0) = foo[1];
    *(int*)(vendor+4) = foo[3];
    *(int*)(vendor+8) = foo[2];

    vendor[12] = '\0';

#if 0 && (defined(i386) || defined(__i386__) || defined(__x86_64__) || defined(__amd64))
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
    //strcpy(vendor, "unknown");
#endif
}

void
pixie_cpu_brand(char model[64])
{
    int foo[4] = {0,0,0,0};

    __cpuid(foo, 0x80000002);
    *(int*)(model+ 0) = foo[0];
    *(int*)(model+ 4) = foo[1];
    *(int*)(model+ 8) = foo[2];
    *(int*)(model+12) = foo[3];
    __cpuid(foo, 0x80000003);
    *(int*)(model+16) = foo[0];
    *(int*)(model+20) = foo[1];
    *(int*)(model+24) = foo[2];
    *(int*)(model+28) = foo[3];
    __cpuid(foo, 0x80000004);
    *(int*)(model+32) = foo[0];
    *(int*)(model+36) = foo[1];
    *(int*)(model+40) = foo[2];
    *(int*)(model+44) = foo[3];

    model[48] = '\0';

#if 0 && (defined(i386) || defined(__i386__) || defined(__x86_64__) || defined(__amd64))
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
    //strcpy(vendor, "unknown");
#endif
}


void pixie_cpu_info(struct CpuInfo *info)
{
    unsigned x;
    int foo[4] = {0,0,0,0};

    __cpuid(foo, 1);
    x = foo[0];

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
}

