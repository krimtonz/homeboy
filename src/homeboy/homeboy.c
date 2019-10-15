#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>

#include "homeboy.h"
#include "sd.h"

static vc_regs_t vc_regs;

static void *n64_dram = NULL;

static memory_domain_t memory = {
        {0,0,0,0},
        NULL, -hb_mmreg,
        lb, lh, lw, ld,
        sb, sh, sw, sd,
        unk_0x2C_,
        hb_mmreg,
        hb_mmreg | 0xFFFF,
        0x00000000, 0x0000FFFF,
};

static void do_write(){
    vc_regs.ready = 0;
    vc_regs.busy = 1;
    uint8_t ret = sdio_write_sectors(vc_regs.write_lba,vc_regs.block_cnt,(void*)((char*)n64_dram + vc_regs.addr));
    vc_regs.error = ret;
    vc_regs.ready = 1;
    vc_regs.busy = 0;
}

static void do_read(){
    vc_regs.ready = 0;
    vc_regs.busy = 1;
    uint8_t ret = sdio_read_sectors(vc_regs.read_lba,vc_regs.block_cnt,(void*)((char*)n64_dram + vc_regs.addr));
    vc_regs.error = ret;
    vc_regs.ready = 1;
    vc_regs.busy = 0;
}

static void do_status_update(){
    if(vc_regs.initialize){
        if(sdio_is_initialized()){
            sdio_stop();
        }
        memset(&vc_regs,0,sizeof(vc_regs));
        vc_regs.busy = 1;
        uint8_t ret = sdio_start();
        vc_regs.error = ret;
        if(ret == SD_ERROR_SUCCESS){
            vc_regs.inserted = sdio_is_inserted();
            vc_regs.sdhc = sdio_is_sdhc();
            vc_regs.ready = 1;
            vc_regs.busy = 0;
            vc_regs.key = 0x1234;
        }
        vc_regs.busy = 0;
    }
}

uint8_t lb(void* callback, uint32_t addr, uint8_t* dest){
    *dest = (uint8_t)vc_regs.regs[addr>>2];
    return 1;
}
uint8_t lh(void* callback, uint32_t addr, uint16_t* dest){
    *dest = (uint16_t)vc_regs.regs[addr>>2];
    return 1;
}
uint8_t lw(void* callback, uint32_t addr, uint32_t* dest){
    *dest = (uint32_t)vc_regs.regs[addr>>2];
    return 1;
}
uint8_t ld(void* callback, uint32_t addr, uint64_t* dest){
    *dest = (uint64_t)vc_regs.regs[addr>>2];
    return 1;
}
uint8_t sb(void* callback, uint32_t addr, uint8_t* src){
    vc_regs.regs[addr>>2] = *src;
    return 1;
}
uint8_t sh(void* callback, uint32_t addr, uint16_t* src){
    vc_regs.regs[addr>>2] = *src;
    return 1;
}

uint8_t sw(void* callback, uint32_t addr, uint32_t* src){
    vc_regs.regs[addr>>2] = *src;
    if(addr==0x08){
        do_write();
    }
    if(addr == 0x0C){
        do_read();
    }
    if(addr==0x14){
        do_status_update();
    }
    return 1;
}
uint8_t sd(void* callback, uint32_t addr, uint64_t* src){
    uint32_t *src32 = (uint32_t*)src;
    vc_regs.regs[addr>>2] = src32[0];
    vc_regs.regs[(addr>>2) + 1] = src32[1];
    return 1;
}

uint8_t unk_0x2C_(void* callback, uint32_t addr, void* unk){
    return 1;
}

ENTRY bool _start(void **dest){

#if VC_VERSION == NACE
    n64_system.mem_index[(hb_mmreg >> 12) & 0xFFFFF] = 0x70;
#else
    n64_system.mem_index[(hb_mmreg >> 16) & 0xFFFF] = 0x70;
#endif
    
    n64_system.memory_domain[0x70] = &memory;
    
    bool ret = n64_dram_alloc(dest,0x0800000);
    if(ret){
        n64_dram = dest[1];
    }
    return ret;
}