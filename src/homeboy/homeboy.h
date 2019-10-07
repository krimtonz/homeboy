#ifndef _HOMEBOY_H
#define _HOMEBOY_H

#include <stdint.h>

#define ENTRY   __attribute__((section(".init")))

#define hb_mmreg    0xA8050000

typedef struct{
    char        unk_0x00_[0x04];                                            /* 0x00 */
    void       *callback_data;                                              /* 0x04 */
    uint32_t    addr_mask;                                                  /* 0x08 */
    uint8_t   (*lb)(void *callback_data, uint32_t addr, uint8_t *dest);     /* 0x0C */
    uint8_t   (*lh)(void *callback_data, uint32_t addr, uint16_t *dest);    /* 0x10 */
    uint8_t   (*lw)(void *callback_data, uint32_t addr, uint32_t *dest);    /* 0x14 */
    uint8_t   (*ld)(void *callback_data, uint32_t addr, uint64_t *dest);    /* 0x18 */
    uint8_t   (*sb)(void *callback_data, uint32_t addr, uint8_t *src);      /* 0x1C */
    uint8_t   (*sh)(void *callback_data, uint32_t addr, uint16_t *src);     /* 0x20 */
    uint8_t   (*sw)(void *callback_data, uint32_t addr, uint32_t *src);     /* 0x24 */
    uint8_t   (*sd)(void *callback_data, uint32_t addr, uint64_t *src);     /* 0x28 */
    uint8_t   (*unk_0x2C_)(void*, uint32_t, void*);                         /* 0x2C */
    uint32_t    n64_vram_start;                                             /* 0x30 */
    uint32_t    n64_vram_end;                                               /* 0x34 */
    uint32_t    n64_pram_start;                                             /* 0x38 */
    uint32_t    n64_pram_end;                                               /* 0x3C */
} memory_domain_t;                                                          /* 0x40 */

typedef struct{
    char                unk_0x00_[0xB60];       /* 0x00000 */
    memory_domain_t    *memory_domain[0x100];   /* 0x00B60 */
    uint8_t             mem_index[0x10000];     /* 0x00F60 */
    void               *recompiler_1;           /* 0x10F60 */
    void               *recompiler_2;           /* 0x10F64 */
} n64_system_t;

typedef union{
    struct{
        uint32_t key;
        uint32_t addr;
        uint32_t write_lba;
        uint32_t read_lba;
        uint32_t block_cnt;
    };
    uint32_t regs[5];

} vc_regs_t;

#define n64_system_addr     0x809F6C34
#define alloc_addr          0x800810f8
#define n64_dram_alloc_addr 0x80041d7c

typedef bool (*alloc_t)(void **dest,uint32_t size);

#define n64_system      (*(n64_system_t*) n64_system_addr)
#define alloc           ((alloc_t) alloc_addr)
#define n64_dram_alloc  ((alloc_t) n64_dram_alloc_addr)

uint8_t lb(void* callback, uint32_t addr, uint8_t* dest);
uint8_t lh(void* callback, uint32_t addr, uint16_t* dest);
uint8_t lw(void* callback, uint32_t addr, uint32_t* dest);
uint8_t ld(void* callback, uint32_t addr, uint64_t* dest);
uint8_t sb(void* callback, uint32_t addr, uint8_t* src);
uint8_t sh(void* callback, uint32_t addr, uint16_t* src);
uint8_t sw(void* callback, uint32_t addr, uint32_t* src);
uint8_t sd(void* callback, uint32_t addr, uint64_t* src);
uint8_t unk_0x2C_(void* callback, uint32_t addr, void* unk);

#endif