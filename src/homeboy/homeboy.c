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
#include "hb_heap.h"

int hb_hid = -1;
static hb_sd_regs_t *homeboy_obj = NULL;

int homeboy_event(hb_sd_regs_t *regs, int event, void *arg);


const class_type_t homeboy_class = 
{
    "HOMEBOY",
    sizeof(hb_sd_regs_t),
    0,
    homeboy_event
};

static void *n64_dram = NULL;
static char dram_fn[64];

static void do_write() 
{
    homeboy_obj->ready = 0;
    homeboy_obj->busy = 1;

    if(sdio_write_sectors(homeboy_obj->write_lba, homeboy_obj->block_cnt, (void*)((char*)n64_dram + homeboy_obj->addr)))
    {
        homeboy_obj->error = SD_ERROR_SUCCESS;
    }
    else
    {
        homeboy_obj->error = SD_ERROR_INVAL;
    }

    homeboy_obj->ready = 1;
    homeboy_obj->busy = 0;
}

static void do_read()
{
    homeboy_obj->ready = 0;
    homeboy_obj->busy = 1;

    if(sdio_read_sectors(homeboy_obj->read_lba, homeboy_obj->block_cnt, (void*)((char*)n64_dram + homeboy_obj->addr)))
    {
        homeboy_obj->error = SD_ERROR_SUCCESS;    
    }
    else
    {
        homeboy_obj->error = SD_ERROR_INVAL;
    }

    homeboy_obj->ready = 1;
    homeboy_obj->busy = 0;
}

 static void do_status_update()
 {
    if(homeboy_obj->reset)
    {
        int fd = fs_open(dram_fn, 3);
        if(fd < 0)
        {
            fd = fs_create(dram_fn, 3);
        }

        if(fd >= 0)
        {
            uint32_t dram_params[2];
            dram_params[0] = homeboy_obj->dram_save;
            dram_params[1] = homeboy_obj->dram_save_len;
            fs_write(fd, dram_params, sizeof(dram_params));
            fs_write(fd, (char*)n64_dram + dram_params[0], dram_params[1]);
            fs_close(fd);
        }
        
        reset_flag = 1;
    }

    if(homeboy_obj->initialize) 
    {
        if(sdio_is_initialized())
        {
            sdio_stop();
        }

        homeboy_obj->busy = 1;
        homeboy_obj->ready = 0;

        if(sdio_start())
        {
            homeboy_obj->error = SD_ERROR_SUCCESS;
            homeboy_obj->inserted = sdio_is_inserted();
            homeboy_obj->sdhc = sdio_is_sdhc();
            homeboy_obj->ready = 1;
        } 
        else
        {
            homeboy_obj->error = SD_ERROR_INVAL;
        }

        homeboy_obj->busy = 0;
    }
}

bool lb(hb_sd_regs_t *hb_regs, uint32_t addr, uint8_t *dest)
{
    addr -= 0x100A0000;
    *dest = (uint8_t)hb_regs->regs[addr >> 2];

    return true;
}

bool lh(hb_sd_regs_t *hb_regs, uint32_t addr, uint16_t *dest)
{
    addr -= 0x100A0000;
    *dest = (uint16_t)hb_regs->regs[addr >> 2];

    return true;
}

bool lw(hb_sd_regs_t *hb_regs, uint32_t addr, uint32_t *dest)
{
    addr -= 0x100a0000;

    if(addr == 0x024)
    {
        *dest = (gettick() & 0xFFFFFFFF00000000) >> 32;
        return true;
    }
    else if(addr == 0x028)
    {
        *dest = gettick() & 0xFFFFFFFF;
        return true;
    }

    *dest = (uint32_t)hb_regs->regs[addr >> 2];
    return true;
}

bool ld(hb_sd_regs_t *hb_regs, uint32_t addr, uint64_t *dest)
{
    addr -= 0x100a0000;
    *dest = (uint64_t)homeboy_obj->regs[addr >> 2];
    return true;
}

bool sb(hb_sd_regs_t *hb_regs, uint32_t addr, uint8_t *src)
{
    addr -= 0x100A0000;
    hb_regs->regs[addr >> 2] = *src;

    return true;
}

bool sh(hb_sd_regs_t *hb_regs, uint32_t addr, uint16_t *src)
{
    addr -= 0x100A0000;
    hb_regs->regs[addr >> 2] = *src;

    return true;
}

bool sw(hb_sd_regs_t *hb_regs, uint32_t addr, uint32_t *src)
{
    addr -= 0x100A0000;
    
    homeboy_obj->regs[addr >> 2] = *src;
    
    if(addr == 0x08) 
    {
        do_write();
    }
    else if(addr == 0x0C)
    {
        do_read();
    }
    else if(addr == 0x14) 
    {
        do_status_update();
    }

    return true;
}

bool sd(hb_sd_regs_t *hb_regs, uint32_t addr, uint64_t *src)
{
    addr -= 0x100A0000;
    uint32_t *src32 = (uint32_t*)src;
    hb_regs->regs[addr >> 2] = src32[0];
    hb_regs->regs[(addr >> 2) + 1] = src32[1];

    return true;
}

int homeboy_event(hb_sd_regs_t *regs, int event, void *arg)
{
    if(event == 0x1002)
    {
        cpuSetDevicePut(n64_cpu, arg, sb, sh, sw, sd);
        cpuSetDeviceGet(n64_cpu, arg, lb, lh, lw, ld);
    }
}

ENTRY bool _start(void **dest, size_t size)
{
    xlObjectMake(&homeboy_obj, NULL, &homeboy_class);
    cpuMapObject(n64_cpu, homeboy_obj, 0x08050000, 0x0805FFFF, 0);

    xlObjectMake(&hb_heap_obj, NULL, &hb_heap_class);
    cpuMapObject(n64_cpu, hb_heap_obj, 0x08060000, 0x08460000, 0);

    if(hb_hid < 0)
    {
        hb_hid = ios_create_heap((void*)ios_heap_addr, HB_HEAPSIZE);
    }

    if(hb_hid >= 0)
    {
        homeboy_obj->key = 0x1234;
    }

    fs_init();

    bool ret = ramSetSize(dest, 0x00800000);
    if(ret)
    {
        n64_dram = dest[1];
    }

    sprintf(dram_fn, "/title/00010001/%8x/data/dram_save", title_id);
    
    // Check if a dram restore needs to be done. 
    int fd = fs_open(dram_fn,1);
    if(fd >= 0)
    {
        uint32_t dram_params[2];
        fs_read(fd, dram_params, sizeof(dram_params));
        fs_read(fd, (char*)n64_dram + dram_params[0], dram_params[1]);
        fs_close(fd);
        fs_delete(dram_fn);
        homeboy_obj->dram_restore_key = 0x6864;
    }

    return ret;
}