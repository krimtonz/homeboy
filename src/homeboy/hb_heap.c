#ifdef HB_HEAP

#include <stdbool.h>
#include "hb_heap.h"

class_hb_heap_t *hb_heap_obj = NULL;
int hb_heap_event(void *heap_p, int event, void *arg);

static class_type_t hb_heap_class = {
    "HB-HEAP",
    sizeof(class_hb_heap_t),
    0,
    hb_heap_event
};


static bool heap_lb(void* callback, uint32_t addr, uint8_t* dest){
    if(hb_heap_obj != NULL && hb_heap_obj->heap_ptr != NULL){
        addr -= 0x100C0000;
        *dest = *(uint8_t*)(hb_heap_obj->heap_ptr + addr);
        return true;
    }
    return false;
}

static bool heap_lh(void* callback, uint32_t addr, uint16_t* dest){
    if(hb_heap_obj != NULL && hb_heap_obj->heap_ptr != NULL){
        addr -= 0x100C0000;
        *dest = *(uint16_t*)(hb_heap_obj->heap_ptr + addr);
        return true;
    }
    return false;
}

static bool heap_lw(void* callback, uint32_t addr, uint32_t* dest){
    if(hb_heap_obj != NULL && hb_heap_obj->heap_ptr != NULL){
        addr -= 0x100C0000;
        *dest = *(uint32_t*)(hb_heap_obj->heap_ptr + addr);
        return true;
    }
    return false;
}

static bool heap_ld(void* callback, uint32_t addr, uint64_t* dest){
    if(hb_heap_obj != NULL && hb_heap_obj->heap_ptr != NULL){
        addr -= 0x100C0000;
        *dest = *(uint64_t*)(hb_heap_obj->heap_ptr + addr);
        return true;
    }
    return false;
}

static bool heap_sb(void* callback, uint32_t addr, uint8_t* src){
    if(hb_heap_obj != NULL && hb_heap_obj->heap_ptr != NULL){
        addr -= 0x100C0000;
        *(uint8_t*)(hb_heap_obj->heap_ptr + addr) = *src;
        return true;
    }
    return false;
}

static bool heap_sh(void* callback, uint32_t addr, uint16_t* src){
    if(hb_heap_obj != NULL && hb_heap_obj->heap_ptr != NULL){
        addr -= 0x100C0000;
        *(uint16_t*)(hb_heap_obj->heap_ptr + addr) = *src;
        return true;
    }
    return false;
}

static bool heap_sw(void* callback, uint32_t addr, uint32_t* src){
    if(hb_heap_obj != NULL && hb_heap_obj->heap_ptr != NULL){
        addr -= 0x100C0000;
        *(uint32_t*)(hb_heap_obj->heap_ptr + addr) = *src;
        return true;
    }
    return false;
}

static bool heap_sd(void* callback, uint32_t addr, uint64_t* src){
    if(hb_heap_obj != NULL && hb_heap_obj->heap_ptr != NULL){
        addr -= 0x100C0000;
        *(uint64_t*)(hb_heap_obj->heap_ptr + addr) = *src;
        return true;
    }
    return false;
}

int hb_heap_event(void *heap_p, int event, void *arg)
{
    class_hb_heap_t *heap = (class_hb_heap_t*)heap_p;
    // device mapped.
    if(event == 0x1002) {
        if(heap->heap_ptr == NULL){
            allocMEM2(&heap->heap_ptr, 0x400000);
            heap->heap_size = 0x00400000;
        }
        cpuSetDevicePut(gSystem->cpu, arg, heap_sb, heap_sh, heap_sw, heap_sd);
        cpuSetDeviceGet(gSystem->cpu, arg, heap_lb, heap_lh, heap_lw, heap_ld);
    }
}

void homeboy_heap_init(void)
{
    xlObjectMake((void**)&hb_heap_obj, NULL, &hb_heap_class);
    cpuMapObject(gSystem->cpu, hb_heap_obj, 0x08060000, 0x08460000, 0);
}

#endif
