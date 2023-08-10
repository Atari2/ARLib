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
class Timer {
    Instant m_start;
    Instant m_last;
    public:
    Timer() : m_start{ PerfClock::now() }, m_last{ m_start } {}
    void reset() {
        m_start = PerfClock::now();
        m_last  = m_start;
    }
    Duration elapsed() { 
        m_last = PerfClock::now();
        return PerfClock::diff(m_start, m_last); 
    }
    Duration elapsed_since_last() {
		auto now = PerfClock::now();
		auto diff = PerfClock::diff(m_last, now);
		m_last = now;
		return diff;
	}
};
}    // namespace ARLib
