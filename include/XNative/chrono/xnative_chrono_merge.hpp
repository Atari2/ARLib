#pragma once
#include "Types.hpp"
#ifdef UNIX_OR_MINGW
    #include "XNative/chrono/xnative_chrono_unix.hpp"
#else
    #include "XNative/chrono/xnative_chrono_windows.hpp"
#endif

#if not defined(CHRONO_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
    #error "Don't include the XNative files directly. Use Chrono.h"
#endif
namespace ARLib {
using TimePoint = int64_t;
using TimeDiff  = int64_t;
class ChronoNative {
    public:
    static TimePoint now();
};
}    // namespace ARLib
