#pragma once
#define CHRONO_INCLUDED__
#include "XNative/chrono/xnative_chrono_merge.hpp"
namespace ARLib {
class Clock {
    public:
    static Nanos now() { return ChronoNative::now(); }
    static TimeDiff diff(Nanos t1, Nanos t2) { return Nanos{ t2.value - t1.value }; }
};
}    // namespace ARLib
