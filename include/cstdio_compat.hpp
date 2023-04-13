#pragma once
#include "Compat.hpp"
#include "Types.hpp"
#include "TypeTraits.hpp"

extern "C"
{
#if not defined(_VA_LIST_DEFINED) and not defined(_VA_LIST)
    #ifndef _MSC_VER
    typedef __builtin_va_list va_list;
    #else
    typedef char* va_list;
    #endif
#endif
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
#define EOF      (-1)
}
namespace ARLib {

using FsChar = ConditionalT<windows_build, wchar_t, char>;

int remove(const FsChar* filename);
int rename(const FsChar* old_filename, const FsChar* new_filename);

int fscanf(FILE* fp, const char* format, ...);
int scanf(const char* format, ...);
int sscanf(const char* str, const char* format, ...);
int fgetc(FILE* fp);
char* fgets(char* str, int n, FILE* fp);
int fputc(int ch, FILE* fp);
int getc(FILE* fp);
int getchar();

size_t filesize(FILE* fp);
FILE* fopen(const FsChar* filename, const char* mode);
int fclose(FILE* fp);
int fseek(FILE* fp, long off, int whence);
size_t ftell(FILE* fp);
size_t fread(void* buffer, size_t size, size_t count, FILE* fp);
size_t fwrite(const void* buffer, size_t size, size_t count, FILE* fp);
int puts(const char* buf);
int fputs(const char* buf, FILE* fp);
int printf(const char* fmt, ...);
int vprintf(const char* fmt, va_list);
int fprintf(FILE* fp, const char* fmt, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t n, const char* format, ...);
int vsnprintf(char* buf, size_t n, const char* format, va_list args);
int scprintf(const char* format, ...);
}    // namespace ARLib
