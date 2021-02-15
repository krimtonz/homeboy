#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "io.h"
#include "cpu.h"
#include "vc.h"

struct isfs
{
	char    filepath[64];                   /* 0x0000 */
	union {
		struct {
			char        filepathOld[64];    /* 0x0000 */
			char        filepathNew[64];    /* 0x0040 */
		} fsrename;                         /* 0x0080 */
		struct {
			uint32_t    owner_id;           /* 0x0000 */
			uint16_t    group_id;           /* 0x0004 */
			char        filepath[64];       /* 0x0006 */
			uint8_t     ownerperm;          /* 0x0046 */
			uint8_t     groupperm;          /* 0x0047 */
			uint8_t     otherperm;          /* 0x0048 */
			uint8_t     attributes;         /* 0x0049 */
			uint8_t     pad0[2];            /* 0x004A */
		} fsattr;                           /* 0x004C */
		struct {
			ioctlv      vector[4];          /* 0x0000 */
			uint32_t    entry_cnt;          /* 0x0020 */
		} fsreaddir;                        /* 0x0024 */
		struct {
			ioctlv      vector[4];          /* 0x0000 */
			uint32_t    usage1;             /* 0x0020 */
			uint8_t     pad0[28];           /* 0x0024 */
			uint32_t    usage2;             /* 0x0040 */
		} fsusage;                          /* 0x0044 */
		struct {
			uint32_t	a;                  /* 0x0000 */
			uint32_t	b;                  /* 0x0004 */
			uint32_t	c;                  /* 0x0008 */
			uint32_t	d;                  /* 0x000C */
			uint32_t	e;                  /* 0x0010 */
			uint32_t	f;                  /* 0x0014 */
			uint32_t	g;                  /* 0x0018 */
		} fsstats;                          /* 0x001C */
	};                                      /* 0x0040 */

	void       *callback;                   /* 0x00C0 */
	void       *callback_data;              /* 0x00C4 */
	uint32_t    functype;                   /* 0x00C8 */
	void       *funcargv[8];                /* 0x00CC */
};                                          /* 0x00EC */

const char *fspath = "/dev/fs";
static int fs_fd = -1;
static bool fs_initialized = 0;
static struct isfs *fs_buf = NULL;

bool fs_init(){
    if(fs_initialized) return 1;

    fs_buf = ios_alloc(hb_hid,sizeof(*fs_buf),32);
    if(!fs_buf){
        return 0;
    }

    if(fs_fd<0){
        fs_fd = ios_open(fspath,0);
    }
    if(fs_fd<0){
        return 0;
    }

    fs_initialized = 1;
    return 1;
}

int fs_read(int fd, void *buf, size_t len){
    char *rbuf = ios_alloc(hb_hid,0x2000,32);
    int ret = 0;
    char *p = buf;
    while(len>0){
        int to_read = len>0x1000 ? 0x2000 : len;
        int read_cnt = ios_read(fd,rbuf,to_read);
        memcpy(p,rbuf,read_cnt);
        len -= read_cnt;
        ret += read_cnt;
        p += read_cnt;
    }
    ios_free(hb_hid,rbuf);
    return ret;
}

int fs_write(int fd, void *buf, size_t len){
    char *wbuf = ios_alloc(hb_hid,0x2000,32);
    int ret = 0;
    const char *p = buf;
    while(len>0){
        int to_write = len>0x1000 ? 0x2000 : len;
        memcpy(wbuf,p,to_write);
        int written = ios_write(fd,wbuf,to_write);
        len -= written;
        ret += written;
        p += written;
    }
    ios_free(hb_hid,wbuf);
    return ret;
}

int fs_close(int fd){
    return ios_close(fd);
}

int fs_create(char *path, int mode){
    int fd = -1;
    fs_buf->fsattr.attributes = 1;
    fs_buf->fsattr.ownerperm = 3;
    fs_buf->fsattr.groupperm = 3;
    fs_buf->fsattr.otherperm = 3;
    memcpy(fs_buf->fsattr.filepath,path,strlen(path) + 1);
    fd = ios_ioctl(fs_fd,9,&fs_buf->fsattr,sizeof(fs_buf->fsattr),NULL,0);
    if(fd == 0){
        fd = ios_open(path,mode);
    }
    return fd;
}

int fs_open(char *path, int mode){
    memcpy(fs_buf->filepath,path,strlen(path) + 1);
    return ios_open(fs_buf->filepath,mode);
}

int fs_delete(char *path){
    memcpy(fs_buf->filepath,path,strlen(path)+1);
	return ios_ioctl(fs_fd,7,fs_buf->filepath,64,NULL,0);
}