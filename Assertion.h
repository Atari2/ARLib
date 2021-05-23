#pragma once
#include "Macros.h"
#include "Compat.h"
#include "std_includes.h"

void abort_arlib();
void assertion_failed__(const char* msg);

#define HARD_ASSERT(val, msg) if (!val) { puts(msg); assertion_failed__(ERRINFO);  } 
#define HARD_ASSERT_FMT(val, fmt, ...) if (!val) { printf(fmt, __VA_ARGS__); assertion_failed__(ERRINFO); } 

#define SOFT_ASSERT(val, msg) if (!val) { puts(msg); puts(ERRINFO); } 
#define SOFT_ASSERT_FMT(val, fmt, ...) if (!val) { printf(fmt, __VA_ARGS__); puts(ERRINFO); }

#define TODO(cls) cls() {todo__();}; static void todo__() { HARD_ASSERT(false, CONCAT(STRINGIFY(cls), " is not implemented yet")) }