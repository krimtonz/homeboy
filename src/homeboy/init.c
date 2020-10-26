#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "homeboy.h"
#include "hb_heap.h"
#include "vc.h"
#include "hb_fat.h"
#include "fs.h"

#define HB_HEAPSIZE         0xD000

ENTRY bool _start(void **dest, size_t size)
{
    homeboy_init();
    homeboy_heap_init();
    homeboy_fat_init();

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