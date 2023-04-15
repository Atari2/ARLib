#ifndef INCLUDED_FROM_OWN_CPP___
    #define INCLUDED_FROM_OWN_CPP___
#endif
#include "XNative/chrono/xnative_chrono_merge.hpp"
namespace ARLib {
Nanos ChronoNative::now() {
    constexpr auto den = 1'000'000'000L;
#ifdef UNIX_OR_MINGW
    auto spec = time_get();
    return Nanos{ (static_cast<int64_t>(spec.tv_sec) * den) + spec.tv_nsec };
#else
    constexpr long long tenmhz = 10'000'000;
    const long long Freq       = query_perf_frequency();
    const long long Ctr        = query_perf_counter();
    if (Freq == tenmhz) {
        constexpr __int64 Multiplier = den / tenmhz;
        return Nanos{ Ctr * Multiplier };
    } else {
        const __int64 Whole = (Ctr / Freq) * den;
        const __int64 Part  = (Ctr % Freq) * den / Freq;
        return Nanos{ Whole + Part };
    }
#endif
}
}    // namespace ARLib
