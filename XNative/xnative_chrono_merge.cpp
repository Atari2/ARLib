#define INCLUDED_FROM_OWN_CPP___
#include "xnative_chrono_merge.h"
#include <chrono>
namespace ARLib {
    TimePoint ChronoNative::now() {
        constexpr auto den = 1'000'000'000;
#if defined(COMPILER_GCC) or defined(COMPILER_CLANG)
        auto spec = time_get();
        return (spec.tv_sec * den) + spec.tv_nsec;
#else
        constexpr long long tenmhz = 10'000'000;
        const long long Freq = query_perf_frequency();
        const long long Ctr = query_perf_counter();
        if (Freq == tenmhz) {
            constexpr __int64 Multiplier = den / tenmhz;
            return Ctr * Multiplier;
        } else {
            const __int64 Whole = (Ctr / Freq) * den;
            const __int64 Part = (Ctr % Freq) * den / Freq;
            return Whole + Part;
        }
#endif
    }
} // namespace ARLib