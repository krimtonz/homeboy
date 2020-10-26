#ifndef _HB_HEAP_H
#define _HB_HEAP_H

#include <stddef.h>
#include "vc.h"

typedef struct {
    char *heap_ptr;
    size_t heap_size;
} class_hb_heap_t;

void homeboy_heap_init(void);

extern class_hb_heap_t *hb_heap_obj;

#endif