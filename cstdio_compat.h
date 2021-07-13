#pragma once
#include "Compat.h"
#include "Types.h"

extern "C" {
#ifdef WINDOWS
#ifndef _FILE_DEFINED
#define _FILE_DEFINED
typedef struct _iobuf {
    void* placeholder;
} FILE;

__declspec(dllimport) FILE* __cdecl __acrt_iob_func(unsigned);

#define stdin  (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))
#endif
#elif not defined(WINDOWS_MINGW)
#ifndef __FILE_defined
#define __FILE_defined 1
typedef struct _IO_FILE FILE;
#endif

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

#define stdin  stdin
#define stdout stdout
#define stderr stderr
#else
typedef struct _iobuf FILE;
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
}

namespace ARLib {
    FILE* fopen(const char* filename, const char* mode);
    int fclose(FILE* fp);
    int fseek(FILE* fp, long off, int whence);
    size_t ftell(FILE* fp);
    size_t fread(void* buffer, size_t size, size_t count, FILE* fp);
    size_t fwrite(const void* buffer, size_t size, size_t count, FILE* fp);
    int puts(const char* buf);
    int fputs(const char* buf, FILE* fp);
    int printf(const char* fmt, ...);
    int fprintf(FILE* fp, const char* fmt, ...);
    int sprintf(char* str, const char* format, ...);
    int snprintf(char* str, size_t n, const char* format, ...);
    int scprintf(const char* format, ...);
} // namespace ARLib
