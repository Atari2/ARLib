#include "cstring_compat.h"

typedef int word;
#define	wsize	sizeof(word)
#define	wmask	(wsize - 1)

size_t strlen(const char* ptr) {
	size_t len = 0;
	while (*(ptr++))
		len++;
	return len;
}
char* strcpy(char* dest, const char* src) {
	while (*dest++ = *src++);
	return dest;
}
char* strncpy(char* dest, const char* src, size_t num) {
	while (--num)
		*dest++ = *src++;
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
	while (*dst++);
	while (*src) { *dst++ = *src++; }
	return dst;
}

int isspace(int c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
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