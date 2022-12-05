#pragma once
#define CHRONO_INCLUDED__
#include "XNative/chrono/xnative_chrono_merge.h"
namespace ARLib {
class Clock {
    public:
    static TimePoint now() { return ChronoNative::now(); }
    static TimeDiff diff(TimePoint t1, TimePoint t2) { return t2 - t1; }
};
}    // namespace ARLib