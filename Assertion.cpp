#include "Assertion.h"
#include "Compat.h"
#include "StackTrace.h"
#include "cstdarg_compat.h"
#include "SourceLocation.h"

#ifdef COMPILER_MSVC
    #include <intrin.h>
#endif
#ifdef DEBUG_NEW_DELETE
    #include <cstdio>
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
void _assert_printf(const ARLib::SourceLocation& loc, const char* fmt, ...) {
    static char buf[2048];
    va_list lst{};
    va_start(lst, fmt);
#ifdef DEBUG_NEW_DELETE
    ::vsnprintf(buf, sizeof(buf), fmt, lst);
#else
    int ret = ARLib::vsnprintf(buf, sizeof(buf), fmt, lst);
    buf[ret] = '\0';
#endif
    va_end(lst);
    PRINT_SOURCE_LOCATION(buf, loc);

}
void _assert_puts(const ARLib::SourceLocation& loc, const char* str) {
    PRINT_SOURCE_LOCATION(str, loc);
#ifdef DEBUG_NEW_DELETE
    ::puts(str);
#else
    ARLib::puts(str);
#endif
}
