#ifndef PIXIE_CPUID_H
#define PIXIE_CPUID_H

struct CpuInfo {
    unsigned stepping;
    unsigned model;
    unsigned family;
    unsigned type;
    const char *codename;
    unsigned codenumber;
};

void
pixie_cpu_vendor(char vendor[16]);

void
pixie_cpu_brand(char model[64]);

void
pixie_cpu_info(struct CpuInfo *info);

#endif
