#include "cstdio_compat.h"
#include "Assertion.h"
#include "arlib_osapi.h"
#include "PrintfImpl.h"
#include <cstdarg>
#include <cstdio>
#ifdef WINDOWS
    #include "Windows/win_native_io.h"
#endif
namespace ARLib {
int remove(const FsChar* filename) {
#ifdef WINDOWS
    return Win32DeleteFileW(filename);
#else
    return ::remove(filename);
#endif
}
int rename(const FsChar* old_filename, const FsChar* new_filename) {
#ifdef WINDOWS
    return Win32RenameFileW(old_filename, new_filename);
#else
    return ::rename(old_filename, new_filename);
#endif
}
int fscanf(FILE* fp, const char* format, ...) {
    va_list argptr{};
    va_start(argptr, format);
    auto ret = ::vfscanf(fp, format, argptr);
    va_end(argptr);
    return ret;
}
int scanf(const char* format, ...) {
    va_list argptr{};
    va_start(argptr, format);
    auto ret = ::vscanf(format, argptr);
    va_end(argptr);
    return ret;
}
int sscanf(const char* str, const char* format, ...) {
    va_list argptr{};
    va_start(argptr, format);
    auto ret = ::vsscanf(str, format, argptr);
    va_end(argptr);
    return ret;
}
int fgetc(FILE* fp) {
#ifdef WINDOWS
    return ReadChar(fp);
#else
    return ::fgetc(fp);
#endif
}
char* fgets(char* str, int n, FILE* fp) {
#ifdef WINDOWS
    return ReadLine(str, n, fp);
#else
    return ::fgets(str, n, fp);
#endif
}
int fputc(int ch, FILE* fp) {
#ifdef WINDOWS
    return WriteChar(static_cast<char>(ch), fp);
#else
    return ::fputc(ch, fp);
#endif
}
int getc(FILE* fp) {
    return ARLib::fgetc(fp);
}
int getchar() {
    return ARLib::fgetc(stdin);
}
size_t filesize(FILE* fp) {
#ifdef WINDOWS
    return Win32SizeFile(fp);
#else
    auto cur = ARLib::ftell(fp);
    ARLib::fseek(fp, SEEK_END, 0);
    auto size = ARLib::ftell(fp);
    ARLib::fseek(fp, SEEK_SET, static_cast<int>(cur));
    return size;
#endif
}
FILE* fopen(const FsChar* filename, const char* mode) {
#ifdef WINDOWS
    return Win32OpenFileW(filename, mode);
#else
    return ::fopen(filename, mode);
#endif
}
int fclose(FILE* fp) {
#ifdef WINDOWS
    return Win32CloseFile(fp);
#else
    return ::fclose(fp);
#endif
}
int fseek(FILE* fp, long off, int whence) {
#ifdef WINDOWS
    return Win32SeekFile(fp, off, whence);
#else
    return ::fseek(fp, off, whence);
#endif
}
size_t ftell(FILE* fp) {
#ifdef WINDOWS
    return Win32TellFile(fp);
#else
    return static_cast<size_t>(::ftell(fp));
#endif
}
size_t fread(void* buffer, size_t size, size_t count, FILE* fp) {
    return ::fread(buffer, size, count, fp);
}
size_t fwrite(const void* buffer, size_t size, size_t count, FILE* fp) {
#ifdef WINDOWS
    return WriteFileGeneric(buffer, size, count, fp);
#else
    return ::fwrite(buffer, size, count, fp);
#endif
}
int puts(const char* buf) {
#ifdef WINDOWS
    return static_cast<int>(WriteStringOutGeneric(buf));
#else
    return ::puts(buf);
#endif
}
int fputs(const char* buf, FILE* fp) {
#ifdef WINDOWS
    return static_cast<int>(WriteStringFileGeneric(buf, fp));
#else
    return ::fputs(buf, fp);
#endif
}
int printf(const char* fmt, ...) {
    va_list argptr{};
    va_start(argptr, fmt);
    auto ret = _vsprintf(fmt, argptr);
    ARLib::fwrite(ret.result.data(), sizeof(char), ret.result.size(), stdout);
    va_end(argptr);
    return ret.written_arguments;
}
int fprintf(FILE* fp, const char* fmt, ...) {
    va_list argptr{};
    va_start(argptr, fmt);
    auto ret = _vsprintf(fmt, argptr);
    ARLib::fwrite(ret.result.data(), sizeof(char), ret.result.size(), fp);
    va_end(argptr);
    return ret.written_arguments;
}
int sprintf(char* str, const char* format, ...) {
    va_list argptr{};
    va_start(argptr, format);
    auto ret = _vsprintf(format, argptr);
    if (ret.error_code != PrintfErrorCodes::Ok) { return -1; }
    ARLib::strncpy(str, ret.result.data(), ret.result.size());
    va_end(argptr);
    return static_cast<int>(ret.result.size());
}
int snprintf(char* str, size_t n, const char* format, ...) {
    va_list argptr{};
    va_start(argptr, format);
    auto ret = _vsprintf(format, argptr);
    if (ret.error_code != PrintfErrorCodes::Ok) { return -1; }
    ARLib::strncpy(str, ret.result.data(), n);
    va_end(argptr);
    return static_cast<int>(ret.result.size());
}
int scprintf(const char* format, ...) {
    va_list argptr{};
    va_start(argptr, format);
#ifdef WINDOWS
    auto ret = ::_vscprintf_l(format, nullptr, argptr);
#else
    auto ret = ::vsnprintf(nullptr, 0, format, argptr);
#endif
    va_end(argptr);
    return ret;
}
}    // namespace ARLib
