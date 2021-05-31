#pragma once
#include "Types.h"
namespace ARLib {
	constexpr inline size_t strlen(const char* ptr) {
		size_t len = 0;
		while (*(ptr++))
			len++;
		return len;
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
	int toupper(int c);
	int tolower(int c);
}