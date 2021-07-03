#include "cstdio_compat.h"
#include "Assertion.h"
#include <cstdarg>
#include <cstdio>
namespace ARLib {

    FILE* fopen(const char* filename, const char* mode) {
#ifdef WINDOWS
        FILE* pfile = nullptr;
        errno_t err = ::fopen_s(&pfile, filename, mode);
        SOFT_ASSERT_FMT((err == 0), "Failed to open %s in mode %s", filename, mode);
        return pfile;
#else
        return ::fopen(filename, mode);
#endif
    }

    int fclose(FILE* fp) { return ::fclose(fp); }

    int fseek(FILE* fp, long off, int whence) { return ::fseek(fp, off, whence); }

    long ftell(FILE* fp) { return ::ftell(fp); }

    size_t fread(void* buffer, size_t size, size_t count, FILE* fp) { return ::fread(buffer, size, count, fp); }

    size_t fwrite(const void* buffer, size_t size, size_t count, FILE* fp) { return ::fwrite(buffer, size, count, fp); }

    int puts(const char* buf) { return ::puts(buf); }

    int fputs(const char* buf, FILE* fp) { return ::fputs(buf, fp); }

    int printf(const char* fmt, ...) {
        va_list argptr;
        va_start(argptr, fmt);
        auto ret = ::vprintf(fmt, argptr);
        va_end(argptr);
        return ret;
    }

    int fprintf(FILE* fp, const char* fmt, ...) {
        va_list argptr;
        va_start(argptr, fmt);
        auto ret = ::vfprintf(fp, fmt, argptr);
        va_end(argptr);
        return ret;
    }

    int sprintf(char* str, const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        auto ret = ::vsprintf(str, format, argptr);
        va_end(argptr);
        return ret;
    }
    int snprintf(char* str, size_t n, const char* format, ...) {
        va_list argptr;
        va_start(argptr, format);
        auto ret = ::vsnprintf(str, n, format, argptr);
        va_end(argptr);
        return ret;
    }
    int scprintf(const char* format, ...) {
        va_list argptr;
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
