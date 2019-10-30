#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "homeboy.h"
#include "sd.h"
#include "fs.h"
#include "cpu.h"

int hb_hid = -1;

static hb_sd_regs_t hb_sd_regs;
static void *n64_dram = NULL;
static char dram_fn[64];

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
    hb_sd_regs.ready = 0;
    hb_sd_regs.busy = 1;
    if(sdio_write_sectors(hb_sd_regs.write_lba,hb_sd_regs.block_cnt,(void*)((char*)n64_dram + hb_sd_regs.addr))){
        hb_sd_regs.error = SD_ERROR_SUCCESS;
    }else{
        hb_sd_regs.error = SD_ERROR_INVAL;
    }
    hb_sd_regs.ready = 1;
    hb_sd_regs.busy = 0;
}

static void do_read(){
    hb_sd_regs.ready = 0;
    hb_sd_regs.busy = 1;
    if(sdio_read_sectors(hb_sd_regs.read_lba,hb_sd_regs.block_cnt,(void*)((char*)n64_dram + hb_sd_regs.addr))){
        hb_sd_regs.error = SD_ERROR_SUCCESS;    
    }else{
        hb_sd_regs.error = SD_ERROR_INVAL;
    }
    hb_sd_regs.ready = 1;
    hb_sd_regs.busy = 0;
}

 static void do_status_update(){
    if(hb_sd_regs.reset){
        int fd = fs_open(dram_fn,3);
        if(fd<0){
            fd = fs_create(dram_fn,3);
        }
        if(fd>=0){
            uint32_t dram_params[2];
            dram_params[0] = hb_sd_regs.dram_save;
            dram_params[1] = hb_sd_regs.dram_save_len;
            fs_write(fd,dram_params,sizeof(dram_params));
            fs_write(fd,(char*)n64_dram + dram_params[0],dram_params[1]);
            fs_close(fd);
        }
        
        reset_flag = 1;
        
    }
    if(hb_sd_regs.initialize){
        if(sdio_is_initialized()){
            sdio_stop();
        }
        hb_sd_regs.busy = 1;
        hb_sd_regs.ready = 0;
        if(sdio_start()){
            hb_sd_regs.error = SD_ERROR_SUCCESS;
            hb_sd_regs.inserted = sdio_is_inserted();
            hb_sd_regs.sdhc = sdio_is_sdhc();
            hb_sd_regs.ready = 1;
        }else{
            hb_sd_regs.error = SD_ERROR_INVAL;
        }
        hb_sd_regs.busy = 0;
    }
}

uint8_t lb(void* callback, uint32_t addr, uint8_t* dest){
    *dest = (uint8_t)hb_sd_regs.regs[addr>>2];
    return 1;
}
uint8_t lh(void* callback, uint32_t addr, uint16_t* dest){
    *dest = (uint16_t)hb_sd_regs.regs[addr>>2];
    return 1;
}
uint8_t lw(void* callback, uint32_t addr, uint32_t* dest){
    if(addr == 0x024){
        *dest = (gettick() & 0xFFFFFFFF00000000) >> 32;
        return 1;
    }else if(addr == 0x028){
        *dest = gettick() & 0xFFFFFFFF;
        return 1;
    }
    *dest = (uint32_t)hb_sd_regs.regs[addr>>2];
    return 1;
}
uint8_t ld(void* callback, uint32_t addr, uint64_t* dest){
    *dest = (uint64_t)hb_sd_regs.regs[addr>>2];
    return 1;
}
uint8_t sb(void* callback, uint32_t addr, uint8_t* src){
    hb_sd_regs.regs[addr>>2] = *src;
    return 1;
}
uint8_t sh(void* callback, uint32_t addr, uint16_t* src){
    hb_sd_regs.regs[addr>>2] = *src;
    return 1;
}

uint8_t sw(void* callback, uint32_t addr, uint32_t* src){
    hb_sd_regs.regs[addr>>2] = *src;
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
    hb_sd_regs.regs[addr>>2] = src32[0];
    hb_sd_regs.regs[(addr>>2) + 1] = src32[1];
    return 1;
}

uint8_t unk_0x2C_(void* callback, uint32_t addr, void* unk){
    return 1;
}

ENTRY bool _start(void **dest){

    if(hb_hid<0){
        hb_hid = ios_create_heap((void*)ios_heap_addr,HB_HEAPSIZE);
    }
    if(hb_hid>=0){
        hb_sd_regs.key = 0x1234;
    }

    fs_init();

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

    sprintf(dram_fn,"/title/00010001/%8x/data/dram_save",title_id);
    
    // Check if a dram restore needs to be done. 
    int fd = fs_open(dram_fn,1);
    if(fd>=0){
        uint32_t dram_params[2];
        fs_read(fd,dram_params,sizeof(dram_params));
        fs_read(fd,(char*)n64_dram + dram_params[0], dram_params[1]);
        fs_close(fd);
        fs_delete(dram_fn);
        hb_sd_regs.dram_restore_key = 0x6864;
    }

    return ret;
}