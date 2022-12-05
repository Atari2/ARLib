#pragma once
#include "Compat.h"
namespace ARLib {
typedef decltype(nullptr) nullptr_t;
#ifdef WINDOWS
typedef int errno_t;
    #if defined(ENVIRON64)
typedef unsigned __int64 size_t;
typedef signed __int64 ssize_t;
typedef unsigned __int64 uintptr_t;
typedef __int64 ptrdiff_t;
typedef __int64 intptr_t;
    #else
        #error "This library is 64-bit only"
    #endif
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int8 int8_t;
#else
// we fall here if we're using MINGW or we're on Linux
    #define SIZED decltype(sizeof(void*))
    #if defined(ENVIRON64)
static_assert(sizeof(long long) == sizeof(void*));
typedef SIZED size_t;
typedef SIZED uintptr_t;
typedef signed long long ssize_t;
typedef long long ptrdiff_t;
typedef long long intptr_t;
    #else
        #error "This library is 64-bit only"
    #endif
template <typename A, typename B, size_t N>
struct __SizePickerImpl {};
template <typename A, typename B>
struct __SizePickerImpl<A, B, 8> {
    using Type = A;
};
template <typename A, typename B>
struct __SizePickerImpl<A, B, 4> {
    using Type = B;
};
struct __I64BitPicker {
    using Type = __SizePickerImpl<long, long long, 8>::Type;
};
struct __U64BitPicker {
    using Type = __SizePickerImpl<unsigned long, unsigned long long, 8>::Type;
};
using __I64BitPickerT = __I64BitPicker::Type;
using __U64BitPickerT = __U64BitPicker::Type;

static_assert(sizeof(long long) == 8);
typedef __U64BitPickerT uint64_t;

static_assert(sizeof(int) == 4);
typedef unsigned int uint32_t;

static_assert(sizeof(short) == 2);
typedef unsigned short uint16_t;

static_assert(sizeof(char) == 1);
typedef unsigned char uint8_t;

typedef __I64BitPickerT int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;
#endif
constexpr size_t BITS_PER_BYTE = 8;
}    // namespace ARLib
