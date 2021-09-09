#pragma once
#include "std_includes.h"

#ifdef COMPILER_MSVC // MSVC
#include <intrin.h>
#define unreachable __assume(0);
#define forceinline __forceinline
#define noop        __noop
#ifdef _WIN64
#define ENVIRON64 1
#else
#define ENVIRON32 1
#endif

#elif defined(COMPILER_GCC) // GCC
#define unreachable __builtin_unreachable();
#define forceinline __attribute__((always_inline))
#define noop        ((void)0)
#if __x86_64__ || __ppc64__
#define ENVIRON64 1
#else
#define ENVIRON32 1
#endif

#elif defined(COMPILER_CLANG) // CLANG
#define unreachable __builtin_unreachable();
#define forceinline __attribute__((always_inline))
#define noop        ((void)0)
#if __x86_64__ || __ppc64__
#define ENVIRON64 1
#else
#define ENVIRON32 1
#endif
#else // OTHER

#error "Unsupported compiler"
#endif

#ifdef _WIN64
#ifdef _MSC_VER
// WINDOWS means strictly MSVC, if you want mingw, use MINDOWS_MINGW
#define WINDOWS
#else
#define WINDOWS_MINGW
#define UNIX_OR_MINGW
#endif
#elif (defined(__unix__) || defined(__unix)) && !defined(BSD)
#define UNIX
#define UNIX_OR_MINGW
#else
#error "Unsupported platform"
#endif

#ifdef ENVIRON64
#ifdef WINDOWS
static_assert(sizeof(void*) == 8 && sizeof(unsigned __int64) == 8);
#else
static_assert(sizeof(void*) == 8 && sizeof(unsigned long long) == 8);
#endif
#else
#error "This library is 64-bit only"
#endif

static_assert(sizeof(char) == 1);

#ifdef COMPILER_MSVC
#define HAS_BUILTIN(builtin) 0
#else
#define HAS_BUILTIN(builtin) __has_builtin(builtin)
#endif