#include <stdint.h>
#include <sdcard/wiisd_io.h>
#include <irq.h>
#include <ipc.h>
#include <stdlib.h>
#include <malloc.h>

#include "homeboy.h"

static vc_regs_t vc_regs;
static void *n64_dram = NULL;
static memory_domain_t memory = {
        {0,0,0,0},
        (void*)0,
        -hb_mmreg,
        lb,
        lh,
        lw,
        ld,
        sb,
        sh,
        sw,
        sd,
        unk_0x2C_,
        hb_mmreg,
        hb_mmreg | 0xFFFF,
        0x00000000,
        0x0000FFFF,
};

void init_stack(void (*hbfunc)(void)){
    register void *r_stack;
    register void *func;
    static char[0x2000] stack;
    func = hbfunc;
    r_stack = &stack[sizeof(stack)];

    __asm__ volatile("stw 1, -0x04(%1);"
                     "mflr 0;"
                     "stw 0, -0x08(%1);"
                     "subi 1, %1, 0x08;"
                     "mtctr %0;"
                     "bctrl;"
                     "lwz 1, -0x04(%1);"
                     "lwz 0, -0x08(%1);"
                     "mtlr 0"
                     ::
                     "r"(hbfunc),
                     "r"(r_stack);
}

static void do_write(){
    __io_wiisd.writeSectors(vc_regs.write_lba,vc_regs.block_cnt,(void*)(n64_dram + vc_regs.addr));
}

static void do_read(){
    __io_wiisd.readSectors(vc_regs.read_lba,vc_regs.block_cnt,(void*)(n64_dram + vc_regs.addr));
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
        init_stack(do_write);
    }
    if(addr == 0x0C){
        init_stack(do_read);
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

void hb_main(){
    __IPC_ClntInit();
    __io_wiisd.startup();
    
    void *buf = malloc(512);
    __io_wiisd.readSectors(0,1,buf);
    

    vc_regs.key = 0x1234;
}

ENTRY bool _init(void **dest){
    init_stack(hb_main);

    n64_system.mem_index[(hb_mmreg >> 16) & 0xFFFF] = 0x70;
    n64_system.memory_domain[0x70] = &memory;
    
    bool ret = n64_dram_alloc(dest,0x0800000);
    if(ret){
        n64_dram = *dest;
    }
    return ret;
}