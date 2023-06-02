#pragma once
#include "std_includes.hpp"

#ifdef COMPILER_MSVC    // MSVC
    #define arlib_unreachable __assume(0);
    #define arlib_forceinline __forceinline
    #define arlib_noop        __noop
    #if _MSC_FULL_VER > 193431942
        #define compiler_intrinsic [[msvc::intrinsic]]
    #else
        #define compiler_intrinsic
    #endif
    #ifdef _WIN64
        #define ENVIRON64 1
    #else
        #define ENVIRON32 1
    #endif

#elif defined(COMPILER_GCC)    // GCC
    #define arlib_unreachable __builtin_unreachable();
    #define arlib_forceinline __attribute__((always_inline))
    #define arlib_noop        ((void)0)
    #define compiler_intrinsic
    #if __x86_64__ || __ppc64__
        #define ENVIRON64 1
    #else
        #define ENVIRON32 1
    #endif

#elif defined(COMPILER_CLANG)    // CLANG
    #define arlib_unreachable __builtin_unreachable();
    #define arlib_forceinline __attribute__((always_inline))
    #define arlib_noop        ((void)0)
    #define compiler_intrinsic
    #if __x86_64__ || __ppc64__
        #define ENVIRON64 1
    #else
        #define ENVIRON32 1
    #endif
#else    // OTHER

    #error "Unsupported compiler"
#endif

#ifdef _WIN64
constexpr bool windows_build = true;
    #ifdef _MSC_VER
        // WINDOWS means strictly MSVC, if you want mingw, use MINDOWS_MINGW
        #define WINDOWS
    #else
        #define WINDOWS_MINGW
        #define UNIX_OR_MINGW
    #endif
#elif (defined(__unix__) || defined(__unix)) && !defined(BSD)
constexpr bool windows_build = false;
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
