#pragma once
#include "../Types.h"
#if defined(COMPILER_GCC) or defined(COMPILER_CLANG)
#include "xnative_chrono_unix.h"
#else
#include "xnative_chrono_windows.h"
#endif

#if not defined(CHRONO_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
#error "Don't include the XNative files directly. Use Chrono.h"
#endif

namespace ARLib {
    using TimePoint = int64_t;
    using TimeDiff = int64_t;
    class ChronoNative {
        public:
        static TimePoint now();
    };
}