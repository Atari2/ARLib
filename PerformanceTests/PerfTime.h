#pragma once
#include "../Console.h"
#include "../Functional.h"
#include "../Macros.h"
#include "../Pair.h"
#include "../String.h"
#include "../Vector.h"

#define TIMER_START(coll)                                                                                              \
    {                                                                                                                  \
        ARLib::RAIIPerfCounter _counter{coll};

#define TIMER_END }

#define PERF_SUITE_START(name)                                                                                         \
    {                                                                                                                  \
        ARLib::detail::PerfSuite name{#name};

#define PERF_SUITE_END }

#define ADD_PERF_START(suite_name, perf_name) suite_name.append_timer(#perf_name, []() TIMER_START(nullptr)

#define ADD_PERF_END TIMER_END );

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
        explicit RAIIPerfCounter(Vector<int64_t>* vec = nullptr);
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