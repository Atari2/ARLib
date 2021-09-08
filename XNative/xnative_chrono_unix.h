#pragma once
#if defined(COMPILER_GCC) or defined(COMPILER_CLANG)
#if not defined(CHRONO_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
#error "Don't include the XNative files directly. Use Use Chrono.h"
#endif

namespace ARLib {
    using time_t = long int;
    struct TimeSpecC {
        time_t tv_sec;
        long tv_nsec;
    };

    TimeSpecC time_get();

}

#endif