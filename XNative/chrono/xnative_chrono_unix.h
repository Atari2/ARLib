#pragma once

#include "../../Compat.h"
#ifdef UNIX_OR_MINGW
#if not defined(CHRONO_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
#error "Don't include the XNative files directly. Use Use Chrono.h"
#endif

namespace ARLib {
#ifndef ON_MINGW
    using time_t = long int;
#else
    using time_t = long long int;
#endif
    struct TimeSpecC {
        time_t tv_sec;
        long tv_nsec;
    };

    TimeSpecC time_get();

} // namespace ARLib

#endif