#ifndef _HOMEBOY_H
#define _HOMEBOY_H

#include <stdint.h>

#define hb_mmreg    0xA8050000

typedef union{
    struct{
        uint32_t key;
        uint32_t addr;
        uint32_t write_lba;
        uint32_t read_lba;
        uint32_t block_cnt;
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
            uint32_t status;
        };
        uint32_t dram_save;
        uint32_t dram_save_len;
        uint32_t dram_restore_key;
        uint32_t timebase_hi;
        uint32_t timebase_lo;
    };
    uint32_t regs[11];
} hb_sd_regs_t;

extern int hb_hid;
void homeboy_init(void);

extern hb_sd_regs_t *homeboy_obj;
extern void *n64_dram;
extern char dram_fn[64];

#endif
