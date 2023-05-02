#pragma once
#define CHRONO_INCLUDED__
#include "XNative/chrono/xnative_chrono_merge.hpp"
namespace ARLib {
class PerfClock {
    public:
    static Instant now() { return ChronoNative::now(); }
    static Duration diff(Instant t1, Instant t2) { return Duration{ Nanos{ t2.raw_value() - t1.raw_value() } }; }
};
class DateClock {
    public:
    static Instant now() { return ChronoNative::datenow(); }
    static Duration diff(Instant t1, Instant t2) { return Duration{ Nanos{ t2.raw_value() - t1.raw_value() } }; }
};
}    // namespace ARLib
