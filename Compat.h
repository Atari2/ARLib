#pragma once

#ifdef _MSC_VER		// MSVC
#include <intrin.h>
#define unreachable __assume(0);
#ifdef _WIN64
	#define ENVIRON64 1
#else
	#define ENVIRON32 1
#endif

#elif __clang__		// CLANG
#define unreachable __builtin_unreachable();
#if __x86_64__ || __ppc64__
	#define ENVIRON64 1
#else
	#define ENVIRON32 1
#endif

#elif __GNUG__		// GCC
#if __x86_64__ || __ppc64__
	#define ENVIRON64 1
#else
	#define ENVIRON32 1
#endif
#define unreachable __builtin_unreachable();

#else				// OTHER
#error "Unsupported compiler"
#endif

#if defined(_WIN32) || defined(_WIN64)
#define WINDOWS
#elif (defined(__unix__) || defined(__unix)) && !defined(BSD)
#define UNIX
#else
#error "Unsupported platform"
#endif

#ifdef ENVIRON64
	#ifdef WINDOWS
		static_assert(sizeof(void*) == 8 && sizeof(unsigned __int64) == 8);
	#else
		static_assert(sizeof(void*) == 8 && sizeof(unsigned long) == 8);
	#endif
#else
	#ifdef WINDOWS
		static_assert(sizeof(void*) == 4 && sizeof(unsigned int) == 4);
	#else
		static_assert(sizeof(void*) == 4 && sizeof(unsigned int) == 4);
	#endif
#endif

static_assert(sizeof(char) == 1);