#include "Assertion.h"
#include "Compat.h"
#include "StackTrace.h"
#ifdef COMPILER_MSVC
#include <intrin.h>
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
