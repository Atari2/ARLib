#include "cstdio_compat.h"
#include <cstdio>
#include <cstdarg>
namespace ARLib {

	FILE* fopen(const char* filename, const char* mode)
	{
		return ::fopen(filename, mode);
	}

	int fclose(FILE* fp) {
		return ::fclose(fp);
	}

	int fseek(FILE* fp, long off, int whence) {
		return ::fseek(fp, off, whence);
	}

	long ftell(FILE* fp) {
		return ::ftell(fp);
	}

	int puts(const char* buf) {
		return ::puts(buf);
	}

	int fputs(const char* buf, FILE* fp) {
		return ::fputs(buf, fp);
	}

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
}
