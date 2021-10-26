#include "cstdio_compat.h"
#include "Assertion.h"
#include "arlib_osapi.h"
#include <cstdarg>
#include <cstdio>
namespace ARLib {

    int remove(const char* filename) { return ::remove(filename); }
    int rename(const char* old_filename, const char* new_filename) { return ::rename(old_filename, new_filename); }

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
    int fgetc(FILE* fp) { return ::fgetc(fp); }
    char* fgets(char* str, int n, FILE* fp) { return ::fgets(str, n, fp); }
    int fputc(int ch, FILE* fp) { return ::fputc(ch, fp); }
    int getc(FILE* fp) { return ::getc(fp); }
    int getchar() { return ::getchar(); }

    FILE* fopen(const char* filename, const char* mode) {
#ifdef WINDOWS
        FILE* pfile = nullptr;
        errno_t err = ::fopen_s(&pfile, filename, mode);
#ifndef DEBUG
        (void)err;
#else
        if (err != 0) print_last_error();
        HARD_ASSERT_FMT((err == 0), "Failed to open %s in mode %s", filename, mode)
#endif
        return pfile;
#else
        return ::fopen(filename, mode);
#endif
    }

    int fclose(FILE* fp) { return ::fclose(fp); }

    int fseek(FILE* fp, long off, int whence) { return ::fseek(fp, off, whence); }

    size_t ftell(FILE* fp) { return static_cast<size_t>(::ftell(fp)); }

    size_t fread(void* buffer, size_t size, size_t count, FILE* fp) { return ::fread(buffer, size, count, fp); }

    size_t fwrite(const void* buffer, size_t size, size_t count, FILE* fp) { return ::fwrite(buffer, size, count, fp); }

    int puts(const char* buf) { return ::puts(buf); }

    int fputs(const char* buf, FILE* fp) { return ::fputs(buf, fp); }

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
