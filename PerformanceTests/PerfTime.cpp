#include "PerfTime.h"

namespace ARLib {

    TimePoint PerfCounter::start() {
        m_last_time = Clock::now();
        return m_last_time;
    }
    Pair<TimePoint, TimeDiff> PerfCounter::stop() {
        auto curr = Clock::now();
        TimeDiff diff = Clock::diff(m_last_time, curr);
        m_last_time = curr;
        return {m_last_time, diff};
    }
    TimePoint PerfCounter::current_time() { return Clock::now(); }
    RAIIPerfCounter::RAIIPerfCounter(Vector<TimeDiff>*& vec) : m_start(m_counter.start()), m_vec(vec) {}
    RAIIPerfCounter::~RAIIPerfCounter() {
        auto [_, diff] = m_counter.stop();
        if (m_vec) m_vec->append(diff);
        Console::print("%lld nanoseconds\n", diff);
    }
} // namespace ARLib