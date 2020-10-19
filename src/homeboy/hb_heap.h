#ifndef _HB_HEAP_H
#define _HB_HEAP_H

#include <stddef.h>
#include "homeboy.h"

typedef struct {
    char *heap_ptr;
    size_t heap_size;
} class_hb_heap_t;

int hb_heap_event(class_hb_heap_t *heap, int event, void *arg);

extern class_hb_heap_t *hb_heap_obj;
extern const class_type_t hb_heap_class;

#endif