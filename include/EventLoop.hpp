#pragma once
#ifndef DISABLE_THREADING
    #include "Functional.hpp"
    #include "Threading.hpp"
    #include "TypeTraits.hpp"
    #include "Vector.hpp"
namespace ARLib {
class EventLoop {
    Vector<Function<void()>> m_callbacks;
    Mutex m_callback_loc;
    Thread m_thread{};
    Atomic<bool> m_running{ false };
    Atomic<bool> m_sleeping{ false };
    ConditionVariable m_condition_var;
    ConditionVariable m_sleep_condition;

    static void loop_function(EventLoop* loop);

    public:
    enum class JoinType {
        WaitUntilFinished,
        ForceStop,
    };
    EventLoop() = default;

    void start();
    void stop() {
        m_condition_var.notify_one();
        m_running = false;
    }
    bool running() { return m_running; }
    template <typename Functor, typename... Args>
    requires CallableWith<Functor, Args...>
    void subscribe_callback(Functor&& func, Args&&... args) {
        auto lam = [f = move(func), ... arguments = move(args)]() {
            f(arguments...);
        };
        {
            ScopedLock lock{ m_callback_loc };
            m_callbacks.append(move(lam));
            if (!m_running) start();
            m_condition_var.notify_one();
        }
    }
    int join(JoinType join_type = JoinType::WaitUntilFinished) {
        if (join_type == JoinType::WaitUntilFinished) {
            UniqueLock lock{ m_callback_loc };
            m_sleep_condition.wait(lock, [&]() { return m_sleeping.load(); });
        }
        stop();
        m_thread.join();
        return 0;
    }
};
}    // namespace ARLib
#endif
