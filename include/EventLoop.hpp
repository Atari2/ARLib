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
    EventLoop()                 = default;
    EventLoop(const EventLoop&) = delete;
    EventLoop(EventLoop&& other) noexcept {
        other.m_thread.swap(m_thread);
        m_callbacks       = move(other.m_callbacks);
        m_running         = other.m_running.load();
        m_sleeping        = other.m_sleeping.load();
        m_condition_var   = move(other.m_condition_var);
        m_sleep_condition = move(other.m_sleep_condition);
    };
    void start();
    void stop() {
        m_running = false;
        m_condition_var.notify_one();
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
