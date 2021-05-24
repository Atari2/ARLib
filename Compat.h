#pragma once

#ifdef _MSC_VER		// MSVC
#define WINDOWS
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