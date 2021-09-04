#define INCLUDED_FROM_OWN_CPP___
#include "xnative_chrono_unix.h"
#ifdef ON_LINUX
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

namespace ARLib {
    
    TimeSpecC time_get() {
        TimeSpecC tp{};
        syscall(SYS_clock_gettime, 0 /* CLOCK_REALTIME */, reinterpret_cast<timespec*>(&tp));
        return tp;
    }
} // namespace ARLib
#endif