#pragma once
#ifndef DISABLE_THREADING
#include "Functional.h"
#include "Threading.h"
#include "TypeTraits.h"
#include "Vector.h"

namespace ARLib {
    class EventLoop {
        Vector<Function<void()>> m_callbacks;
        Mutex m_callback_loc;
        Thread m_thread{};
        bool m_running{false};

        static void loop_function(EventLoop* loop);

        public:
        EventLoop() = default;

        void start();
        void stop() { m_running = false; }
        bool running() { return m_running; }
        template <typename Functor, typename... Args>
        requires CallableWith<Functor, Args...>
        void subscribe_callback(Functor&& func, Args&&... args) {
            auto lam = [f = move(func), ...arguments = move(args)]() {
                f(arguments...);
            };
            {
                ScopedLock lock{m_callback_loc};
                m_callbacks.append(move(lam));
                if (!m_running) start();
            }
        }
        int join() {
            m_thread.join();
            return 0;
        }
    };
} // namespace ARLib
#endif