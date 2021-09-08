#define INCLUDED_FROM_OWN_CPP___
#include "xnative_chrono_windows.h"
#ifdef COMPILER_MSVC

#include <xtimec.h>

namespace ARLib {

    template <typename U, typename T>
    U _to_(T t) {
        return reinterpret_cast<U>(t);
    }

    int __cdecl time_get(XTimeC* time, int flag) { return xtime_get(_to_<xtime*>(time), flag); }

    long __cdecl time_diff_to_millis(const XTimeC* time) { return _Xtime_diff_to_millis(_to_<const xtime*>(time)); }
    long __cdecl time_diff_to_millis2(const XTimeC* first, const XTimeC* second) {
        return _Xtime_diff_to_millis2(_to_<const xtime*>(first), _to_<const xtime*>(second));
    }
    long long __cdecl time_get_ticks() { return _Xtime_get_ticks(); }

    long long __cdecl query_perf_counter() { return _Query_perf_counter(); }
    long long __cdecl query_perf_frequency() { return _Query_perf_frequency(); }
} // namespace ARLib

#endif