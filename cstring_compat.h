#pragma once
#include "Types.h"
#include <immintrin.h>
namespace ARLib {
	constexpr inline size_t strlen(const char* ptr) {
		size_t len = 0;
		while (*(ptr++))
			len++;
		return len;
	}

	template <size_t NUM>
	char* strncpy_vectorized(char* dest, const char* src) {
		static_assert(NUM % 32 == 0, "Size of strncpy vectorized must be a multiple of 32");
		for (size_t base = 0; base < NUM; base += 32) {
			__m256i buffer = _mm256_loadu_si256((const __m256i*)(src + base));
			_mm256_storeu_si256((__m256i*)(dest + base), buffer);
		}
		return dest;
	}

	char* strcpy(char* dest, const char* src);
	char* strncpy(char* destination, const char* source, size_t num);
	int strcmp(const char*, const char*);
	int strncmp(const char*, const char*, size_t);
	char* strcat(char* dst, const char* src);
	int isspace(int c);
	int isdigit(int c);
	int isalnum(int c);
	void* memmove(void* dst, const void* src, size_t num);
	void* memcpy(void* dst, const void* src, size_t num);
	int toupper(int c);
	int tolower(int c);
}
