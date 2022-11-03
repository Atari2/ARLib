#pragma once
#include "../Compat.h"
#ifdef WINDOWS
#include "../cstring_compat.h"
extern "C" {
typedef struct _iobuf FILE;
}
namespace ARLib {
    // Read
    char ReadChar(FILE* fp);
    char* ReadLine(char* buf, int n, FILE* fp);
    char ReadInChar();
    size_t ReadFileGeneric(void* buffer, size_t size, size_t count, FILE* fp);
    
    // Write
    size_t WriteFileGeneric(const void* buffer, size_t size, size_t count, FILE* fp);
    size_t WriteOutGeneric(const void* buffer, size_t size, size_t count);
    size_t WriteStringFileGeneric(const char* buf, FILE* fp);
    size_t WriteStringOutGeneric(const char* buf);
    bool WriteChar(char c, FILE* fp);

    // File handling
    FILE* Win32OpenFile(const char* filename, const char* mode);
    bool Win32CloseFile(FILE* fp);
    bool Win32DeleteFile(const char* filename);
    bool Win32RenameFile(const char* filename_old, const char* filename_new);
    int Win32SeekFile(FILE* fp, int off, int whence);
    size_t Win32TellFile(FILE* fp);
    size_t Win32SizeFile(FILE* fp);
}
#endif