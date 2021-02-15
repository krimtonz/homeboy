#include <string.h>
#include <sys/stat.h>
#include "cpu.h"
#include "sys.h"
#include "vc.h"
#include "stdio.h"
#include "hb_exception.h"
#include "hb_heap.h"

#ifdef HB_EXCEPTIONS

static int waiting = 1;
static thread_context_t *faulting_thread = NULL;
static thread_context_t faulting_copy;

static thread_context_t dump_thread;
static thread_context_t wait_thread;
static char dump_stack[0x10000];
static char wait_stack[0x1000];

#define write_f(f,fmt,...)                                      \
    do {                                                        \
        snprintf(str_buf, sizeof(str_buf), fmt, __VA_ARGS__);   \
        write(f, str_buf, strlen(str_buf) + 1);                 \
    } while(0)

void ppchalt(void) {
    asm("sync\n");
    while(1) {
        asm("nop\n"
            "li 3,0\n"
            "nop\n");
    }
}

void *wait_for_mem_dump(void *arg) {
    while(waiting) {
        OSSuspendThread(&wait_thread);
    }
    return NULL;
}

void *dump_mem_thread(void *arg) {
    static char str_buf[256];
    static char title_id_buf[5];

    snprintf(title_id_buf, sizeof(title_id_buf), "%s", (char*)&title_id);
    title_id_buf[4] = 0;

    snprintf(str_buf, sizeof(str_buf), "/hb_%s_mem1_crash_dump.bin", title_id_buf);
    int file = creat(str_buf, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    write(file, (void*)0x80000000, 0x1800000);
    close(file);

    snprintf(str_buf, sizeof(str_buf), "/hb_%s_crash_details.txt", title_id_buf);
    file = creat(str_buf, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    write_f(file, "Faulting PC: %08x\n", faulting_copy.srr0);
    for(int i = 0, c = 0; i < 32; i++) {
        if(c == 3 || i == 31) {
            write_f(file, "GPR %d: %08x\n", i, faulting_copy.gpr[i]);
            c = 0;
        } else { 
            write_f(file, "GPR %d: %08x\t", i, faulting_copy.gpr[i]);
            c++;
        }
    }
    for(int i = 0, c = 0; i < 32; i++) {
        if(c == 1 || i == 31) {
            write_f(file, "FPR %d: %016x\n", i, faulting_copy.fpr_64[i]);
            c = 0;
        } else {
            write_f(file, "FPR %d: %016x\t", i, faulting_copy.fpr_64[i]);
            c++;
        }
    }

    write_f(file, "CR: %08x\n", faulting_copy.cr);
    write_f(file, "LR: %08x\n", faulting_copy.lr);
    write_f(file, "CTR: %08x\n", faulting_copy.ctr);
    write_f(file, "XER: %08x\n", faulting_copy.xer);
    write_f(file, "FSCR: %016x\n", faulting_copy.fscr);
    write_f(file, "SRR0: %08x\n", faulting_copy.srr0);
    write_f(file, "SRR1: %08x\n", faulting_copy.srr1);

    close(file);

#ifdef HB_HEAP
    snprintf(str_buf, sizeof(str_buf), "/hb_%s_hb_heap_dump.bin", title_id_buf);
    file = creat(str_buf, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(hb_heap_obj != NULL) {
        write(file, hb_heap_obj->heap_ptr, hb_heap_obj->heap_size);
    } else {
        char *str = "hb_heap_obj was null, no dump made.";
        write(file, str, strlen(str));
    }
    close(file);
#endif

    waiting = 0;
    ppchalt();
    return NULL;
}

void handle_exception(enum ppc_exception exception) {
    faulting_thread = (thread_context_t*)MEM_PHYSICAL_TO_K0(cur_thread);
    faulting_copy = *faulting_thread;

    OSCreateThread(&dump_thread, dump_mem_thread, NULL, dump_stack + sizeof(dump_stack), sizeof(dump_stack), 30, 0);
    OSCreateThread(&wait_thread, wait_for_mem_dump, NULL, wait_stack + sizeof(wait_stack), sizeof(wait_stack), 30, 0);
    OSResumeThread(&wait_thread);
    OSResumeThread(&dump_thread);
    OSSuspendThread(faulting_thread);
    
}

void init_hb_exceptions(void) {
    const enum ppc_exception exceptions_to_handle[] = {
        EX_DSI, EX_ISI, EX_FP_UNAVAIL, EX_PROG
    };

    for(int i = 0; i < sizeof(exceptions_to_handle) / sizeof(*exceptions_to_handle); i++) {
        ex_handlers[i] = __handle_exception;
    }
}

#else
void handle_exception(enum ppc_exception exception) {
}

void init_hb_exceptions(void) {
}

#endif
