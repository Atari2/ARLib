#include "Assertion.h"
#include "Compat.h"

void abort__() {
#ifdef _MSC_VER
	__fastfail(1);
#else
	asm volatile("call abort");
#endif
	unreachable
}

void abort_arlib() {
	abort__();
}

void assertion_failed__(const char* msg) {
	puts(msg);
	abort_arlib();
}