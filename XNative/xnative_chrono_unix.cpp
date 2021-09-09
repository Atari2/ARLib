#define INCLUDED_FROM_OWN_CPP___
#include "xnative_chrono_unix.h"
#ifdef UNIX_OR_MINGW
#ifndef ON_MINGW
#include <sys/syscall.h>
#endif
#include <sys/time.h>
#include <unistd.h>

namespace ARLib {

    TimeSpecC time_get() {
        TimeSpecC tp{};
#ifndef ON_MINGW
        syscall(SYS_clock_gettime, 0 /* CLOCK_REALTIME */, reinterpret_cast<timespec*>(&tp));
#else
        clock_gettime(0 /* CLOCK_REALTIME */, reinterpret_cast<timespec*>(&tp));
#endif
        return tp;
    }
} // namespace ARLib
#endif