#pragma once
#include "Compat.h"

extern "C" {
#ifdef WINDOWS
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
	typedef struct _iobuf
	{
		void* placeholder;
	} FILE;

	__declspec(dllimport) FILE* __cdecl __acrt_iob_func(unsigned);

	#define stdin  (__acrt_iob_func(0))
	#define stdout (__acrt_iob_func(1))
	#define stderr (__acrt_iob_func(2))
#endif
#else
#ifndef __FILE_defined
#define __FILE_defined 1
	typedef struct _IO_FILE FILE;
#endif

	extern FILE* stdin;
	extern FILE* stdout;
	extern FILE* stderr;

#define stdin stdin
#define stdout stdout
#define stderr stderr
#endif

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

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
