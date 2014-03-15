#include "bench.h"
#include "pixie-rdtsc.h"

void
bench_asm_ptr(void)
{
    void *v;
    unsigned long long start, stop;
    unsigned i;
    
    /*
     * Create a pointer that points to itself. Thus, as we follow this
     * pointer, we don't actually change it.
     */
    v = (void*)&v;
    
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
#elif defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov eax, i
        mov ebx, v

again:
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]
        mov ebx, dword ptr[ebx]

        dec eax
        jg again
    };
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
        double clocks = elapsed/30.0/ITERATIONS;
        printf("asm-ptr = %5.3f-clocks\n", clocks);
    }
}
