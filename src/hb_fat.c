#include <stdint.h>
#include <sys/stat.h>
#include "vc.h"
#include "sys.h"
#include "homeboy.h"
#include "hb_heap.h"

#if HB_FAT

#define get_n64_buf ((void*)(n64_dram + (((uint32_t)hb_fat_obj->n64_buffer) & 0x3FFFFFF)))

enum {
    SYS_OPEN,
    SYS_OPEN_DIR,
    SYS_CLOSE_DIR,
    SYS_READ_DIR,
    SYS_CREAT,
    SYS_WRITE,
    SYS_READ,
    SYS_CLOSE,
    SYS_TIME,
    SYS_GET_CWD,
    SYS_CHDIR,
    SYS_FSTAT,
    SYS_FSTAT_AT,
    SYS_ISATTY,
    SYS_LSEEK,
    SYS_TRUNC,
    SYS_RENAME,
    SYS_CHMOD,
    SYS_UNLINK,
    SYS_SEEK_DIR,
    SYS_TELL_DIR,
    SYS_REWIND_DIR,
    SYS_RM_DIR,
    SYS_MK_DIR,
    SYS_STAT,
    SYS_LSTAT
};

typedef struct {
    union {
        struct {
            uint32_t command;
            uint32_t n64_buffer;
            union {
                struct {
                    char *path;
                    uint32_t open_flags;
                    uint32_t has_mode;
                    uint32_t mode;
                } open;
                struct {
                    char *dir;
                } opendir;
                struct {
                    void *dir;
                } close_dir;
                struct {
                    void *dir;
                    dirent_t *buf;
                } read_dir;
                struct {
                    char *path;
                    uint32_t mode;
                } creat;
                struct {
                    int fd;
                    void *buf;
                    uint32_t byte_cnt;
                } write;
                struct {
                    int fd;
                    void *buf;
                    uint32_t byte_cnt;
                } read;
                struct {
                    int fd;
                } close;
                struct {
                    void *buf;
                    size_t size;
                } getcwd;
                struct {
                    char *path;
                    struct stat *stat;
                } stat;
            };
        };
        uint32_t regs[6];
    };
} hb_fat_class_t;

static hb_fat_class_t *hb_fat_obj = NULL;

int hb_fat_event(void *hb_fat_p, int event, void *arg);

static class_type_t hb_fat_class = {
    "HB-FAT",
    sizeof(hb_fat_class_t),
    0,
    hb_fat_event
};

void run_command()
{
    switch(hb_fat_obj->command)
    {
        case SYS_OPEN:
        {
            char *path = n64_dram + (((uint32_t)hb_fat_obj->open.path) & 0x3FFFFFF);
            if(hb_fat_obj->open.has_mode){
                *(uint32_t*)get_n64_buf = open(path, hb_fat_obj->open.open_flags, hb_fat_obj->open.mode);
            } else {
                *(uint32_t*)get_n64_buf = open(path, hb_fat_obj->open.open_flags);
            }
            break;
        }
        case SYS_OPEN_DIR:
        {
            char *path = n64_dram + (((uint32_t)hb_fat_obj->opendir.dir) & 0x3FFFFFF);
            *(DIR**)get_n64_buf = opendir(path);
            break;
        }
        case SYS_CLOSE_DIR:
        {
            *(int*)get_n64_buf = closedir(hb_fat_obj->close_dir.dir);
        }
        case SYS_READ_DIR:
        {
            dirent_t *dirent = readdir(hb_fat_obj->read_dir.dir);
            if(dirent != NULL){
                dirent_t *n64_dirent = n64_dram + (((uint32_t)hb_fat_obj->read_dir.buf) & 0x3FFFFFF);
                *n64_dirent = *dirent;
                *(int*)get_n64_buf = 1;
            } else {
                *(int*)get_n64_buf = 0;
            }
            break;
        }
        case SYS_CREAT:
        {
            char *path = n64_dram + (((uint32_t)hb_fat_obj->creat.path) & 0x3FFFFFF);
            *(int*)get_n64_buf = creat(path, hb_fat_obj->creat.mode);
            break;
        }
        case SYS_WRITE:
        {
            void *buf = hb_fat_obj->write.buf;
            if((uint32_t)buf >= HB_HEAP_START) {
                buf = (char*)hb_heap_obj->heap_ptr + ((uint32_t)buf - HB_HEAP_START);
            } else {
                buf = (char*)n64_dram + ((uint32_t)buf & 0x3FFFFFF);
            }
            *(int*)get_n64_buf = write(hb_fat_obj->write.fd, buf, hb_fat_obj->write.byte_cnt);
            break;
        }
        case SYS_READ:
        {
            void *buf = hb_fat_obj->read.buf;
            if((uint32_t)buf >= HB_HEAP_START) {
                buf = (char*)hb_heap_obj->heap_ptr + ((uint32_t)buf - HB_HEAP_START);;
            } else {
                buf = (char*)n64_dram + ((uint32_t)buf & 0x3FFFFFF);
            }
            *(int*)get_n64_buf = read(hb_fat_obj->read.fd, buf, hb_fat_obj->read.byte_cnt);
            break;
        }
        case SYS_CLOSE:
        {
            *(int*)get_n64_buf = close(hb_fat_obj->close.fd);
            break;
        }
        case SYS_GET_CWD:
        {
            char *buf = n64_dram + (((uint32_t)hb_fat_obj->getcwd.buf) & 0x3FFFFFF);
            getcwd(buf, hb_fat_obj->getcwd.size);
            break;
        }
        case SYS_STAT:
        {
            char *path = n64_dram + (((uint32_t)hb_fat_obj->stat.path) & 0x3FFFFFF);
            struct stat *buf = n64_dram + (((uint32_t)hb_fat_obj->stat.stat) & 0x3FFFFFF);
            *(int*)get_n64_buf = stat(path, buf);
            break;
        }

    }
}

static bool lb(hb_fat_class_t *hb_fat, uint32_t addr, uint8_t *dst)
{
    return false;
}

static bool lh(hb_fat_class_t *hb_fat, uint32_t addr, uint16_t *dst)
{
    return false;
}

static bool lw(hb_fat_class_t *hb_fat, uint32_t addr, uint32_t *dst)
{
    return true;
}

static bool ld(hb_fat_class_t *hb_fat, uint32_t addr, uint64_t *dst)
{
    return false;
}

static bool sb(hb_fat_class_t *hb_fat, uint32_t addr, uint8_t *src)
{
    return false;
}

static bool sh(hb_fat_class_t *hb_fat, uint32_t addr, uint16_t *src)
{
    return false;
}

static bool sw(hb_fat_class_t *hb_fat, uint32_t addr, uint32_t *src)
{
    addr &= 0x7FFF;
    hb_fat->regs[addr >> 2] = *src;
    if(addr == 0)
    {
        run_command();
    }
    return true;
}

static bool sd(hb_fat_class_t *hb_fat, uint32_t addr, uint64_t *src)
{
    return false;
}

int hb_fat_event(void *hb_fat_p, int event, void *arg){
    if(event == 0x1002)
    {
        cpuSetDevicePut(gSystem->cpu, arg, sb, sh, sw, sd);
        cpuSetDeviceGet(gSystem->cpu, arg, lb, lh, lw, ld);
    }
}

void homeboy_fat_init(void)
{
    xlObjectMake((void**)&hb_fat_obj, NULL, &hb_fat_class);
    cpuMapObject(gSystem->cpu, hb_fat_obj, 0x8058000, 0x805BFFF, 0);
}

#endif
