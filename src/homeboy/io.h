#ifndef _IO_H
#define _IO_H

#include <stdint.h>

typedef struct
{
	void       *data;
	uint32_t    len;
} ioctlv;

#endif