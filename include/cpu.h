#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>

#define SYS_BASE_CACHED (0x80000000)

#define MEM_PHYSICAL_TO_K0(a) (void*)((uint32_t)(a) + SYS_BASE_CACHED)

inline uint64_t gettick(){
    register uint32_t tbu;
    register uint32_t tbl;
        __asm__ __volatile__ (
            "mftbl %0\n"
            "mftbu %1\n"
            ::
            "r"(tbl), "r"(tbu));
    return (uint64_t)((uint64_t)tbu << 32 | tbl);
}

#endif