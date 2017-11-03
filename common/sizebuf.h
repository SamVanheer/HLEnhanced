#ifndef COMMON_SIZEBUF_H
#define COMMON_SIZEBUF_H

#include "extdll.h"

struct sizebuf_t
{
	const char* buffername;
	unsigned short flags;
	byte* data;
	int maxsize;
	int cursize;
};

#endif //COMMON_SIZEBUF_H
