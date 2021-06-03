#include "cstring_compat.h"
namespace ARLib {
	typedef int word;
#define	wsize	sizeof(word)
#define	wmask	(wsize - 1)

	void* memcpy_vectorized(void* dst0, const void* src0, size_t num) {
		char* dst = static_cast<char*>(dst0);
		const char* src = static_cast<const char*>(src0);
		size_t rem = num % 32;
		for (size_t offset = 0; offset < (num - rem); offset += 32) {
			__m256i buffer = _mm256_loadu_si256((const __m256i*)(src + offset));
			_mm256_storeu_si256((__m256i*)(dst + offset), buffer);
		}
		dst += (num - rem);
		src += (num - rem);
		for (size_t i = 0; i < rem; i++) {
			*dst++ = *src++;
		}
		return dst;
	}

	char* strcpy(char* dest, const char* src) {
		while ((*dest++ = *src++));
		return dest;
	}

	char* strncpy(char* dest, const char* src, size_t num) {
		while (num--) {
			*dest++ = *src++;
			if (*src == '\0')
				break;
		}
		while (num--) {
			*dest++ = '\0';
		}
		return dest;
	}
	int strcmp(const char* first, const char* second) {
		for (; *first == *second; first++, second++)
			if (*first == '\0')
				return 0;
		return *first - *second;
	}
	int strncmp(const char* first, const char* second, size_t num) {
		for (; *first == *second && --num; first++, second++)
			if (*first == '\0')
				return 0;
		return *first - *second;
	}

	char* strcat(char* dst, const char* src) {
		if (*dst != '\0')
			while (*++dst);
		while (*src) { *dst++ = *src++; }
		*dst = *src;
		return dst;
	}

	int isspace(int c) {
		return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
	}

	int isdigit(int c) {
		return c >= 48 && c <= 57;
	}

	int isalnum(int c) {
		return isdigit(c) || (c >= 65 && c <= 90) || (c >= 97 && c <= 122);
	}

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4311 4302)
#endif
	// stolen from an old stdlib implementation
	// assumes that sizeof(char) == 1... should probably use uint8_t but eh
	void* memmove(void* dst0, const void* src0, size_t length) {
		char* dst = static_cast<char*>(dst0);
		const char* src = static_cast<const char*>(src0);
		if (length == 0 || dst == src)
			return dst;
		if ((unsigned long)dst < (unsigned long)src) {
			// copy forward
			size_t t = (size_t)src;
			if ((t | (size_t)dst) & wmask) {
				if ((t ^ (size_t)dst) & wmask || length < wsize)
					t = length;
				else
					t = wsize - (t & wmask);
				length -= t;
				do { *dst++ = *src++; } while (--t);
			}
			t = length / wsize;
			if (t) {
				do { *(word*)dst = *(word*)src; src += wsize; dst += wsize; } while (--t);
			}
			t = length & wmask;
			if (t) {
				do { *dst++ = *src++; } while (--t);
			}
		}
		else {
			src += length;
			dst += length;
			size_t t = (size_t)src;
			if ((t | (size_t)dst) & wmask) {
				if ((t ^ (size_t)dst) & wmask || length <= wsize)
					t = length;
				else
					t &= wmask;
				length -= t;
				do { *--dst = *--src; } while (--t);
			}
			t = length / wsize;
			if (t) {
				do { src -= wsize; dst -= wsize; *(word*)dst = *(word*)src; } while (--t);
			}
			t = length & wmask;
			if (t) {
				do { *--dst = *--src; } while (--t);
			}
		}
		return dst0;
	}
#ifdef _MSC_VER
#pragma warning( pop )
#endif
	void* memcpy(void* dst0, const void* src0, size_t num) {
		[[unlikely]] if (num >= 64) {
			return memcpy_vectorized(dst0, src0, num);
		}
		char* dst = static_cast<char*>(dst0);
		const char* src = static_cast<const char*>(src0);
		while (num--) {
			*dst++ = *src++;
		}
		return dst;
	}
	int toupper(int c) {
		if (c >= 96 && c <= 122)
			return c - 32;
		return c;
	}
	int tolower(int c) {
		if (c >= 65 && c <= 90)
			return c + 32;
		return c;
	}
}
#undef word
#undef wmask