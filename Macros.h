#pragma once
#include "Compat.h"
#include "TypeInfo.h"

#define STRINGIFY(TOK)       #TOK
#define TOSTRING(X)          STRINGIFY(X)
#define CONCAT_IMPL(A, B)    A " " B
#define CONCAT_NL_IMPL(A, B) A "\n" B
#define CONCAT(A, B)         CONCAT_IMPL(A, B)
#define CONCAT_NL(A, B)      CONCAT_NL_IMPL(A, B)

#define LINEINFO CONCAT(TOSTRING(At line), TOSTRING(__LINE__))
#define FILEINFO CONCAT(TOSTRING(In file), __FILE__)

#define ERRINFO CONCAT_NL(LINEINFO, FILEINFO)

#ifdef DEBUG
#define DEBUGMSG(msg)         ARLib::puts(msg);
#define DEBUGFMTMSG(msg, ...) ARLib::printf(msg, __VA_ARGS__);
#else
#define DEBUGMSG(msg)         noop;
#define DEBUGFMTMSG(msg, ...) noop;
#endif

#define COMPTIME_ASSERT(msg)                                                                                           \
    []<bool flag = false>() {                                                                                          \
        static_assert(flag, msg);                                                                                      \
    }                                                                                                                  \
    ();                                                                                                                \
    unreachable

#define CONSTEVAL_STATIC_ASSERT(c, msg)                                                                                \
    static_assert(is_constant_evaluated(),                                                                             \
                  "Only use consteval static assert in constant evaluted contexts, it will leak memory otherwise");    \
    do {                                                                                                               \
        if (!(c)) new char[0];                                                                                         \
    } while (false)
