#include "cstdio_compat.h"
#include "Assertion.h"
#include "arlib_osapi.h"
#include <cstdarg>
#include <cstdio>
#ifdef WINDOWS
#include "Windows/win_native_io.h"
#endif
namespace ARLib {

    int remove(const char* filename) {
#ifdef WINDOWS
        return Win32DeleteFile(filename);
#else
        return ::remove(filename);
#endif
    }
    int rename(const char* old_filename, const char* new_filename) {
#ifdef WINDOWS
        return Win32RenameFile(old_filename, new_filename);
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
        return 0;
#endif
    }

    FILE* fopen(const char* filename, const char* mode) {
#ifdef WINDOWS
        return Win32OpenFile(filename, mode);
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
        return WriteStringFileGeneric(buf, fp);
#else
        return ::fputs(buf, fp);
#endif
    }

    int printf(const char* fmt, ...) {
        va_list argptr{};
        va_start(argptr, fmt);
        auto ret = ::vprintf(fmt, argptr);
        va_end(argptr);
        return ret;
    }

    int fprintf(FILE* fp, const char* fmt, ...) {
        va_list argptr{};
        va_start(argptr, fmt);
        auto ret = ::vfprintf(fp, fmt, argptr);
        va_end(argptr);
        return ret;
    }

    int sprintf(char* str, const char* format, ...) {
        va_list argptr{};
        va_start(argptr, format);
        auto ret = ::vsprintf(str, format, argptr);
        va_end(argptr);
        return ret;
    }
    int snprintf(char* str, size_t n, const char* format, ...) {
        va_list argptr{};
        va_start(argptr, format);
        auto ret = ::vsnprintf(str, n, format, argptr);
        va_end(argptr);
        return ret;
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
} // namespace ARLib
