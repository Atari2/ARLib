#pragma once
size_t strlen(const char*);
char* strcpy(char* dest, const char* src);
char* strncpy(char* destination, const char* source, size_t num);
int strcmp(const char*, const char*);
int strncmp(const char*, const char*, size_t);
char* strcat(char* dst, const char* src);
int isspace(int c);
void* memmove(void* dst, const void* src, size_t num);
int toupper(int c);
int tolower(int c);