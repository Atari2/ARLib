#pragma once

#include "Result.h"
#include "String.h"
#include "EnumHelpers.h"
#include "Variant.h"
#include "Array.h"

// these defines help intellisense
#ifndef _In_z_
    #define _In_z_
#endif
#ifndef _Printf_format_string_
    #define _Printf_format_string_
#endif

#if not defined(_VA_LIST_DEFINED) and not defined(_VA_LIST)
    #ifndef _MSC_VER
typedef __builtin_va_list va_list;
    #else
typedef char* va_list;
    #endif
#endif
namespace ARLib {
MAKE_FANCY_ENUM(PrintfErrorCodes, Ok, InvalidFormat, MissingType, InvalidType, NotImplemented);
struct PrintfBuffer {
    char* buffer;
    size_t buffer_size;
    size_t written_size;
};
enum class PrintfResultType { FromBuffer, FromString };
struct PrintfResult {
    PrintfResultType type;
    Variant<String, PrintfBuffer> result;
    PrintfErrorCodes error_code;
    int written_arguments;

    void reserve(size_t);
    PrintfResult& operator+=(const String& other);
    PrintfResult& operator+=(char other);
    size_t size() const;
    const char* data() const;
    void finalize();
};
PrintfResult _vsprintf(_In_z_ _Printf_format_string_ const char* fmt, va_list l);
PrintfResult _vsprintf_frombuf(_In_z_ _Printf_format_string_ const char* fmt, va_list l, PrintfBuffer buffer);
}    // namespace ARLib
