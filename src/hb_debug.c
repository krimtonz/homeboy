#include <sys/stat.h>
#include "hb_debug.h"
#include "homeboy.h"
#include "hb_heap.h"
#include "vc.h"
#include "sys.h"

#ifdef HB_DBG

enum dbg_cmd {
    DBG_DUMP_MEM
};

typedef struct {
    uint32_t dbg_cmd;
} hb_dbg_class_t;

static hb_dbg_class_t *hb_dbg_obj = NULL;

int hb_debug_event(void *hb_dbg_p, int event, void *arg);

static class_type_t hb_dbg_class = { 
    "HB-DBG",
    sizeof(hb_dbg_class_t),
    0,
    hb_debug_event
};

static void run_cmd(void) {
    switch(hb_dbg_obj->dbg_cmd) {
        case DBG_DUMP_MEM:
        {
            int file = creat("/kz_mem1_dump.bin", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            write(file, (void*)0x80000000, 0x1800000);
            close(file);
            file = creat("/kz_hb_heap_dump.bin", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            write(file, hb_heap_obj->heap_ptr, hb_heap_obj->heap_size);
            close(file);
        }
        break;
    }
}

static bool lb(hb_dbg_class_t *hb_fat, uint32_t addr, uint8_t *dst)
{
    return false;
}

static bool lh(hb_dbg_class_t *hb_fat, uint32_t addr, uint16_t *dst)
{
    return false;
}

static bool lw(hb_dbg_class_t *hb_fat, uint32_t addr, uint32_t *dst)
{
    return true;
}

static bool ld(hb_dbg_class_t *hb_fat, uint32_t addr, uint64_t *dst)
{
    return false;
}

static bool sb(hb_dbg_class_t *hb_fat, uint32_t addr, uint8_t *src)
{
    return false;
}

static bool sh(hb_dbg_class_t *hb_fat, uint32_t addr, uint16_t *src)
{
    return false;
}

static bool sw(hb_dbg_class_t *hb_fat, uint32_t addr, uint32_t *src)
{
    addr &= 0x7FFF;
    if(addr == 0) {
        hb_dbg_obj->dbg_cmd = *src;
        run_cmd();
    }
    return true;
}

static bool sd(hb_dbg_class_t *hb_fat, uint32_t addr, uint64_t *src)
{
    return false;
}

int hb_debug_event(void *hb_dbg_p, int event, void *arg) {
    if(event == 0x1002) {
        cpuSetDevicePut(gSystem->cpu, arg, sb, sh, sw, sd);
        cpuSetDeviceGet(gSystem->cpu, arg, lb, lh, lw, ld);
    }
}

void homeboy_debug_init(void) {
    xlObjectMake((void**)&hb_dbg_obj, NULL, &hb_dbg_class);
    cpuMapObject(gSystem->cpu, hb_dbg_obj, 0x805C000, 0x805FFFF, 0);
}

#endif