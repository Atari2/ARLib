#define INCLUDED_FROM_OWN_CPP___
#include "xnative_chrono_windows.h"
#include "../../Conversion.h"
#ifdef WINDOWS

#include <xtimec.h>

namespace ARLib {

    int __cdecl time_get(XTimeC* time, int flag) { return xtime_get(cast<xtime*>(time), flag); }

    long __cdecl time_diff_to_millis(const XTimeC* time) { return _Xtime_diff_to_millis(cast<const xtime*>(time)); }
    long __cdecl time_diff_to_millis2(const XTimeC* first, const XTimeC* second) {
        return _Xtime_diff_to_millis2(cast<const xtime*>(first), cast<const xtime*>(second));
    }
    long long __cdecl time_get_ticks() { return _Xtime_get_ticks(); }

    long long __cdecl query_perf_counter() { return _Query_perf_counter(); }
    long long __cdecl query_perf_frequency() { return _Query_perf_frequency(); }
} // namespace ARLib

#endif