#ifndef _FS_H
#define _FS_H

#include <stdint.h>
#include "homeboy.h"
#include "io.h"

#define FS_ERROR_EACCES         -1
#define FS_ERROR_EEXIST         -2
#define FS_ERROR_EINVAL         -4
#define FS_ERROR_ENOENT         -6
#define FS_ERROR_EBUSY          -8
#define FS_ERROR_EIO            -12
#define FS_ERROR_ENOMEM         -22
#define FS_ERROR_EINVAL2        -101
#define FS_ERROR_EACCESS        -102
#define FS_ERROR_EIO2           -103
#define FS_ERROR_EEXIST2        -105
#define FS_ERROR_ENOENT2        -106
#define FS_ERROR_ENFILE         -107
#define FS_ERROR_UNK108         -108
#define FS_ERROR_ENFILE2        -109
#define FS_ERROR_ENAMETOOLONG   -110
#define FS_ERROR_UNK111         -111
#define FS_ERROR_EIO3           -114
#define FS_ERROR_ENOTEMPTY      -115
#define FS_ERROR_ENAMETOOLONG2  -116
#define FS_ERROR_EBUSY2         -118
#define FS_ERROR_FATAL          -119

bool fs_init();

int fs_read(int fd, void *buf, size_t len);
int fs_write(int fd, void *buf, size_t len);
int fs_close(int fd);
int fs_create(char *path, int mode);
int fs_open(char *path, int mode);
int fs_delete(char *path);

#endif