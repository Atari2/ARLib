#ifdef ENABLED_TIMING
#include "PerfTime.h"

#include <chrono>

#define NOW std::chrono::high_resolution_clock::now().time_since_epoch().count();

namespace ARLib {

    int64_t PerfCounter::start() {
        m_last_time = NOW;
        return m_last_time;
    }
    Pair<int64_t, int64_t> PerfCounter::stop() {
        auto curr = NOW;
        int64_t diff = curr - m_last_time;
        m_last_time = curr;
        return {m_last_time, diff};
    }
    int64_t PerfCounter::current_time() { return NOW; }
    RAIIPerfCounter::RAIIPerfCounter(Vector<int64_t>* vec) : m_start(m_counter.start()), m_vec(vec) {}
    RAIIPerfCounter::~RAIIPerfCounter() {
        auto [end, diff] = m_counter.stop();
        if (m_vec) m_vec->append(diff);
        ARLib::printf("Timer started at %lld and ended at %lld, total time spent: %lld nanoseconds\n", m_start, end, diff);
    }
} // namespace ARLib
#endif