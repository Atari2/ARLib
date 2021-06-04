#pragma once
#include "Compat.h"

extern "C" {
#ifdef WINDOWS
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
	typedef struct _IO_FILE FILE;
#endif
#else
#ifndef __FILE_defined
#define __FILE_defined 1
	typedef struct _IO_FILE FILE;
#endif
#endif

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

	extern FILE* stdin;
	extern FILE* stdout;
	extern FILE* stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr
}

namespace ARLib {
	FILE* fopen(const char* filename, const char* mode);
	int fclose(FILE* fp);
	int fseek(FILE* fp, long off, int whence);
	long ftell(FILE* fp);
	int puts(const char* buf);
	int fputs(const char* buf, FILE* fp);
	int printf(const char* fmt, ...);
	int fprintf(FILE* fp, const char* fmt, ...);
}