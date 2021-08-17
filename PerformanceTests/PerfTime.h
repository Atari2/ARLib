#pragma once
#include "../Pair.h"
#include "../Vector.h"

#define TIMER_START(coll)                                                                                              \
    {                                                                                                                  \
        ARLib::RAIIPerfCounter __counter{coll};

#define TIMER_END }

namespace ARLib {
    class PerfCounter {
        int64_t m_last_time;

        public:
        int64_t start();
        Pair<int64_t, int64_t> stop();
        int64_t current_time();
    };

    class RAIIPerfCounter {
        PerfCounter m_counter{};
        int64_t m_start;
        Vector<int64_t>* m_vec;
        public:
        RAIIPerfCounter(Vector<int64_t>* vec = nullptr);
        ~RAIIPerfCounter();
    };
} // namespace ARLib