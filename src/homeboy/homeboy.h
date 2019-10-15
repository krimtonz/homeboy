#ifndef _HOMEBOY_H
#define _HOMEBOY_H

#include <stdint.h>

#ifndef VC_VERSION
#error no vc version specified
#endif

#define NACJ 0
#define NACE 1
#define NARJ 2
#define NARE 3

#define ENTRY   __attribute__((section(".init")))

#define hb_mmreg    0xA8050000

#define SD_ERROR_SUCCESS    0
#define SD_ERROR_INVAL      1
#define SD_ERROR_QUEUEFULL  2
#define SD_ERROR_NOMEM      3
#define SD_ERROR_NOBUFFER   4
#define SD_ERROR_OTHER      5

typedef struct{
    char        unk_0x00_[0x04];                                            /* 0x00 */
    void       *callback_data;                                              /* 0x04 */
    uint32_t    addr_mask;                                                  /* 0x08 */
    uint8_t   (*lb)(void *callback_data, uint32_t addr, uint8_t *dest);     /* 0x0C */
    uint8_t   (*lh)(void *callback_data, uint32_t addr, uint16_t *dest);    /* 0x10 */
    uint8_t   (*lw)(void *callback_data, uint32_t addr, uint32_t *dest);    /* 0x14 */
    uint8_t   (*ld)(void *callback_data, uint32_t addr, uint64_t *dest);    /* 0x18 */
    uint8_t   (*sb)(void *callback_data, uint32_t addr, uint8_t *src);      /* 0x1C */
    uint8_t   (*sh)(void *callback_data, uint32_t addr, uint16_t *src);     /* 0x20 */
    uint8_t   (*sw)(void *callback_data, uint32_t addr, uint32_t *src);     /* 0x24 */
    uint8_t   (*sd)(void *callback_data, uint32_t addr, uint64_t *src);     /* 0x28 */
    uint8_t   (*unk_0x2C_)(void*, uint32_t, void*);                         /* 0x2C */
    uint32_t    n64_vram_start;                                             /* 0x30 */
    uint32_t    n64_vram_end;                                               /* 0x34 */
    uint32_t    n64_pram_start;                                             /* 0x38 */
    uint32_t    n64_pram_end;                                               /* 0x3C */
} memory_domain_t;                                                          /* 0x40 */

typedef struct{
    char                unk_0x00_[0xB60];       /* 0x00000 */
    memory_domain_t    *memory_domain[0x100];   /* 0x00B60 */
#if VC_VERSION==NACE
    uint8_t             mem_index[0x100000];    /* 0x00F60 */
#else
    uint8_t             mem_index[0x10000];     /* 0x00F60 */
#endif
    void               *recompiler_1;           /* 0x10F60 */
    void               *recompiler_2;           /* 0x10F64 */
} n64_system_t;

typedef union{
    struct{
        uint32_t key;
        uint32_t addr;
        uint32_t write_lba;
        uint32_t read_lba;
        uint32_t block_cnt;
            union {
                struct{
                uint32_t            : 23;
                uint32_t error      : 4;
                uint32_t initialize : 1;
                uint32_t sdhc       : 1;
                uint32_t inserted   : 1;
                uint32_t busy       : 1;
                uint32_t ready      : 1;
            };
            uint32_t status;
        };
    };
    uint32_t regs[6];
} vc_regs_t;

uint8_t lb(void* callback, uint32_t addr, uint8_t* dest);
uint8_t lh(void* callback, uint32_t addr, uint16_t* dest);
uint8_t lw(void* callback, uint32_t addr, uint32_t* dest);
uint8_t ld(void* callback, uint32_t addr, uint64_t* dest);
uint8_t sb(void* callback, uint32_t addr, uint8_t* src);
uint8_t sh(void* callback, uint32_t addr, uint16_t* src);
uint8_t sw(void* callback, uint32_t addr, uint32_t* src);
uint8_t sd(void* callback, uint32_t addr, uint64_t* src);
uint8_t unk_0x2C_(void* callback, uint32_t addr, void* unk);

#if VC_VERSION == NACJ
#define init_hook_addr          0x800078E8
#define ios_openasync_addr      0x800b884c
#define ios_open_addr           0x800b8964
#define ios_closeasync_addr     0x800b8a84
#define ios_close_addr          0x800b8b44
#define ios_readasync_addr      0x800b8bec
#define ios_read_addr           0x800b8cec
#define ios_writeasync_addr     0x800b8df4
#define ios_write_addr          0x800b8ef4
#define ios_seekasync_addr      0x800b8ffc
#define ios_seek_addr           0x800b90dc
#define ios_ioctlasync_addr     0x800b91c4
#define ios_ioctl_addr          0x800b92fc
#define ios_ioctlvasync_addr    0x800b9568
#define ios_ioctlv_addr         0x800b964c
#define ios_create_heap_addr    0x800b9810
#define ios_alloc_addr          0x800b9b44
#define ios_free_addr           0x800b9b48
#define n64_dram_alloc_addr     0x80041d7c
#define alloc_addr              0x800810f8
#define heap_size_hook_addr     0x8008A164
#define n64_system_addr         0x809F6C34
#elif VC_VERSION == NACE
#define init_hook_addr          0x800078E8
#define ios_openasync_addr      0x800b8858
#define ios_open_addr           0x800b8970
#define ios_closeasync_addr     0x800b8a90
#define ios_close_addr          0x800b8b50
#define ios_readasync_addr      0x800b8bf8
#define ios_read_addr           0x800b8cf8
#define ios_writeasync_addr     0x800b8e00
#define ios_write_addr          0x800b8f00
#define ios_seekasync_addr      0x800b9008
#define ios_seek_addr           0x800b90e8
#define ios_ioctlasync_addr     0x800b91d0
#define ios_ioctl_addr          0x800b9308
#define ios_ioctlvasync_addr    0x800b9574
#define ios_ioctlv_addr         0x800b9658
#define ios_create_heap_addr    0x800b981c
#define ios_alloc_addr          0x800b9b50
#define ios_free_addr           0x800b9b54
#define n64_dram_alloc_addr     0x80041d98
#define alloc_addr              0x80081104
#define heap_size_hook_addr     0x8008a170
#define n64_system_addr         0x809F6d64
#elif VC_VERSION == NARJ
#define ios_openasync_addr      0x800c5430
#define ios_open_addr           0x800c5548
#define ios_closeasync_addr     0x800c5668
#define ios_close_addr          0x800c5728
#define ios_readasync_addr      0x800c57d0
#define ios_read_addr           0x800c58d0
#define ios_writeasync_addr     0x800c59d8
#define ios_write_addr          0x800c5ad8
#define ios_seekasync_addr      0x800c5be0
#define ios_seek_addr           0x800c5cc0
#define ios_ioctlasync_addr     0x800c5da8
#define ios_ioctl_addr          0x800c5ee0
#define ios_ioctlvasync_addr    0x800c614c
#define ios_ioctlv_addr         0x800c6230
#define ios_create_heap_addr    0x800c6608
#define ios_alloc_addr          0x800c693c
#define ios_free_addr           0x800c6940
#define n64_dram_alloc_addr     0x8005083c
#define alloc_addr              0x800887e0
#define n64_system_addr         0x809A7f18
#elif VC_VERSION == NARE
#define ios_openasync_addr      0x800c4dec
#define ios_open_addr           0x800c4f04
#define ios_closeasync_addr     0x800c5024
#define ios_close_addr          0x800c50e4
#define ios_readasync_addr      0x800c518c
#define ios_read_addr           0x800c528c
#define ios_writeasync_addr     0x800c5394
#define ios_write_addr          0x800c5494
#define ios_seekasync_addr      0x800c559c
#define ios_seek_addr           0x800c567c
#define ios_ioctlasync_addr     0x800c5764
#define ios_ioctl_addr          0x800c589c
#define ios_ioctlvasync_addr    0x800c5b08
#define ios_ioctlv_addr         0x800c5bec
#define ios_create_heap_addr    0x800c5fc4
#define ios_alloc_addr          0x800c62f8
#define ios_free_addr           0x800c62fc
#define n64_dram_alloc_addr     0x800507c8
#define alloc_addr              0x80088790
#define n64_system_addr         0x809A3098
#endif

typedef int     (*ios_create_heap_t)(void *heap, size_t size);
typedef void*   (*ios_alloc_t)(int hid, size_t size, size_t page_size);
typedef bool    (*ios_free_t)(int hid, void *ptr);
typedef int     (*ios_openasync_t)(const char *file, int mode, void *callback, void *callback_data);
typedef int     (*ios_open_t)(const char *file, int mode);
typedef int     (*ios_closeasync_t)(int fd, void *callback, void *callback_data);
typedef int     (*ios_close_t)(int fd);
typedef int     (*ios_readasync_t)(int fd, void *data, size_t len, void *callback, void *callback_data);
typedef int     (*ios_read_t)(int fd, void *data, size_t len);
typedef int     (*ios_writeasync_t)(int fd, void *data, size_t len, void *callback, void *callback_data);
typedef int     (*ios_write_t)(int fd, void *data, size_t len);
typedef int     (*ios_seekasync_t)(int fd, int where, int whence, void *callback, void *callback_data);
typedef int     (*ios_seek_t)(int fd, int where, int whence);
typedef int     (*ios_ioctlasync_t)(int fd, int ioctl, void *buffer_in, size_t size_in, void *buffer_io, size_t size_out, void *callback, void *callback_data);
typedef int     (*ios_ioctl_t)(int fd, int ioctl, void *buffer_in, size_t size_in, void *buffer_io, size_t size_out);
typedef int     (*ios_ioctlvasync_t)(int fd, int ioctl, int cnt_in, int cnt_io, void *argv, void *callback, void *callback_data);
typedef int     (*ios_ioctlv_t)(int fd, int ioctl, int cnt_in, int cnt_io, void *argv);
typedef bool    (*alloc_t)(void **dest,uint32_t size);

#define n64_system      (*(n64_system_t*)       n64_system_addr)

#define ios_openasync   ((ios_openasync_t)      ios_openasync_addr)
#define ios_open        ((ios_open_t)           ios_open_addr)
#define ios_closeasync  ((ios_closeasync_t)     ios_closeasync_addr)
#define ios_close       ((ios_close_t)          ios_close_addr)
#define ios_readasync   ((ios_readasync_t)      ios_readasync_addr)
#define ios_read        ((ios_read_t)           ios_read_addr)
#define ios_writeasync  ((ios_writeasync_t)     ios_writeasync_addr)
#define ios_write       ((ios_write_t)          ios_write_addr)
#define ios_seekasync   ((ios_seekasync_t)      ios_seekasync_addr)
#define ios_seek        ((ios_seek_t)           ios_seek_addr)
#define ios_ioctlasync  ((ios_ioctlasync_t)     ios_ioctlasync_addr)
#define ios_ioctl       ((ios_ioctl_t)          ios_ioctl_addr)
#define ios_ioctlvasync ((ios_ioctlvasync_t)    ios_ioctlvasync_addr)
#define ios_ioctlv      ((ios_ioctlv_t)         ios_ioctlv_addr)

#define ios_create_heap ((ios_create_heap_t)    ios_create_heap_addr)
#define ios_alloc       ((ios_alloc_t)          ios_alloc_addr)
#define ios_free        ((ios_free_t)           ios_free_addr)

#define n64_dram_alloc  ((alloc_t)              n64_dram_alloc_addr)

#endif