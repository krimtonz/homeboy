#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>

#define _CPU_ISR_Disable( _isr_cookie ) \
  { register uint32_t _disable_mask = 0; \
	_isr_cookie = 0; \
    __asm__ __volatile__ ( \
	  "mfmsr %0\n" \
	  "rlwinm %1,%0,0,17,15\n" \
	  "mtmsr %1\n" \
	  "extrwi %0,%0,1,16" \
	  : "=&r" ((_isr_cookie)), "=&r" ((_disable_mask)) \
	  : "0" ((_isr_cookie)), "1" ((_disable_mask)) \
	); \
  }

#define _CPU_ISR_Restore( _isr_cookie )  \
  { register uint32_t _enable_mask = 0; \
	__asm__ __volatile__ ( \
    "    cmpwi %0,0\n" \
	"    beq 1f\n" \
	"    mfmsr %1\n" \
	"    ori %1,%1,0x8000\n" \
	"    mtmsr %1\n" \
	"1:" \
	: "=r"((_isr_cookie)),"=&r" ((_enable_mask)) \
	: "0"((_isr_cookie)),"1" ((_enable_mask)) \
	); \
  }

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

void DCInvalidateRange(void *addr, size_t len);
void ICInvalidateRange(void *addr, size_t len);

#endif