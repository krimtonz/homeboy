#ifndef _HOMEBOY_H
#define _HOMEBOY_H

#include <stdint.h>

#define hb_mmreg    0xA8050000

typedef union{
    struct{
        uint32_t key;   /* 0x0000 */
        uint32_t addr;  /* 0x0004 */ 
        uint32_t write_lba; /* 0x0008 */
        uint32_t read_lba;  /* 0x000C */
        uint32_t block_cnt; /* 0x0010 */
        union {
            struct{
                uint32_t                : 22;
                uint32_t reset          : 1;
                uint32_t error          : 4;
                uint32_t initialize     : 1;
                uint32_t sdhc           : 1;
                uint32_t inserted       : 1;
                uint32_t busy           : 1;
                uint32_t ready          : 1;
            };
            uint32_t status;    /* 0x0014 */
        };
        uint32_t dram_save;     /* 0x0018 */
        uint32_t dram_save_len; /* 0x001C */
        uint32_t dram_restore_key;  /* 0x0020 */
        uint32_t timebase_hi;   /* 0x0024 */
        uint32_t timebase_lo;   /* 0x0028 */
    };
    uint32_t regs[11];
} hb_sd_regs_t;

extern int hb_hid;
void homeboy_init(void);

extern hb_sd_regs_t *homeboy_obj;
extern void *n64_dram;
extern char dram_fn[64];

#endif