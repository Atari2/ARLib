#pragma once
#include "../Console.h"
#include "../Functional.h"
#include "../Macros.h"
#include "../Pair.h"
#include "../String.h"
#include "../UniquePtr.h"
#include "../Vector.h"
#include "../Chrono.h"

#define TIMER_START(coll)                                                                                              \
    {                                                                                                                  \
        ARLib::RAIIPerfCounter _counter{coll};

#define TIMER_START_NOARGS                                                                                            \
    {                                                                                                                  \
        ARLib::RAIIPerfCounter _counter{};

#define TIMER_END }

#define PERF_SUITE_START(name)                                                                                         \
    {                                                                                                                  \
        ARLib::detail::PerfSuite name{#name};

#define PERF_SUITE_END }

#define ADD_PERF_START(suite_name, perf_name) suite_name.append_timer(#perf_name, []() TIMER_START_NOARGS

#define ADD_PERF_END TIMER_END );

namespace ARLib {
    class PerfCounter {
        TimePoint m_last_time;

        public:
        TimePoint start();
        Pair<TimePoint, TimeDiff> stop();
        TimePoint current_time();
    };

    class RAIIPerfCounter {
        PerfCounter m_counter{};
        TimePoint m_start;
        UniquePtr<Vector<TimeDiff>> m_vec;

        public:
        explicit RAIIPerfCounter(Vector<TimeDiff>*& vec);
        explicit RAIIPerfCounter() : m_start(0), m_vec() {}
        ~RAIIPerfCounter();
    };

    namespace detail {

        class TimerOnce {
            Function<void()> m_work{};
            String m_timer_name{};

            template <size_t N>
            void do_call(const char (&suite_name)[N]) {
                Console::print("\tFunction \"%s\" from suite \"%s\" -> ", m_timer_name.data(), suite_name);
                m_work();
            }

            public:
            TimerOnce() = default;
            template <size_t N>
            TimerOnce(const char (&timer_name)[N], Function<void()>&& work) :
                m_work(move(work)), m_timer_name(timer_name) {}

            const String& name() { return m_timer_name; }

            template <size_t N>
            void operator()(const char (&suite_name)[N]) {
                do_call(suite_name);
            }
        };

        template <size_t N>
        class PerfSuite {
            char m_name[N]{};
            Vector<TimerOnce> timers{};

            public:
            constexpr explicit PerfSuite(const char (&name)[N]) { ConditionalBitCopy(m_name, name, N); }

            template <size_t M>
            void append_timer(const char (&subname)[M], Function<void()>&& func) {
                timers.append(TimerOnce{subname, Forward<Function<void()>>(func)});
            }
            ~PerfSuite() {
                Console::print("Starting suite \"%s\":\n", m_name);
                for (auto& timer : timers) {
                    timer(m_name);
                }
            }
        };
    } // namespace detail

} // namespace ARLib