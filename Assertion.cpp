#include "Assertion.h"
#include "Compat.h"
#include "StackTrace.h"
#ifdef COMPILER_MSVC
    #include <intrin.h>
#endif
#ifdef DEBUG_NEW_DELETE
    #include <cstdio>
    #include <cstdarg>
#else
    #include "cstdio_compat.h"
#endif
[[noreturn]] void abort__() {
#ifdef _MSC_VER
    __fastfail(1);
#else
    __asm__ volatile("call abort");
#endif
    unreachable
}
void abort_arlib() {
    abort__();
}
void assertion_failed__() {
    BACKTRACE();
    abort_arlib();
}
void _assert_printf(const char* fmt, ...) {
    va_list lst{};
    va_start(lst, fmt);
#ifdef DEBUG_NEW_DELETE
    ::vprintf(fmt, lst);
#else
    ARLib::vprintf(fmt, lst);
#endif
    va_end(lst);
}
void _assert_puts(const char* str) {
#ifdef DEBUG_NEW_DELETE
    ::puts(str);
#else
    ARLib::puts(str);
#endif
}
