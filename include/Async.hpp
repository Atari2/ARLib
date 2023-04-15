#pragma once
#include "Compat.hpp"
#include "Concepts.hpp"
#include "Threading.hpp"
#include "Printer.hpp"
#include "Chrono.hpp"
namespace ARLib {

enum class FutureStatus { Deferred, Ready, Timeout };
template <typename T, bool Deferred, typename Functor, typename... Args>
class Future {
    struct Empty {};
    constexpr static inline bool IsTVoid = IsVoid<T>::value;

    Thread m_executor_thread;
    Atomic<bool> m_result_ready{ false };
    ConditionVariable m_cv{};
    ConditionalT<IsTVoid, Empty, UniquePtr<T>> m_result;
    Mutex m_mutex;


    public:
    Future(Functor&& func, Args&&... args) {
        m_executor_thread = Thread{ [this, f = move(func)]<typename... LArgs>(LArgs&&... largs) {
                                       if constexpr (IsTVoid) {
                                           invoke(f, Forward<LArgs>(largs)...);
                                       } else {
                                           T intermediate = invoke(f, Forward<LArgs>(largs)...);
                                           {
                                               ScopedLock lock{ m_mutex };
                                               m_result = UniquePtr{ move(intermediate) };
                                           }
                                       }
                                       m_cv.notify_all();
                                       m_result_ready.store(true);
                                   },
                                    Forward<Args>(args)... };
    }
    T wait() {
        if (!m_result_ready && m_executor_thread.joinable()) { m_executor_thread.join(); }
        if constexpr (!IsTVoid) { return *m_result; }
    }
    FutureStatus wait_for(Nanos ns) {
        UniqueLock<Mutex> lock{ m_mutex };
        if (m_cv.wait_for(lock, ns, [&]() { return m_result_ready.load(); })) { return FutureStatus::Ready; }
        return FutureStatus::Timeout;
    }
    bool result_ready() const { return m_result_ready.load(); }
    T result() const { return *m_result; }
    ~Future() {
        if (m_executor_thread.joinable()) m_executor_thread.detach();
    }
};
template <typename T, typename Functor, typename... Args>
class Future<T, true, Functor, Args...> {
    PartialFunction<Functor, Args...> m_func;
    UniquePtr<T> m_result;
    public:
    Future(Functor&& func, Args&&... args) :
        m_func{ make_partial_function(Forward<Functor>(func), Forward<Args>(args)...) }, m_result{ nullptr } {}
    T wait() {
        m_result = UniquePtr{ m_func() };
        return *m_result;
    }
    FutureStatus wait_for([[maybe_unused]] Nanos ns) { return FutureStatus::Deferred; }
    bool result_ready() const { return m_result.exists(); }
    T result() const { return *m_result; }
};
template <typename Functor, typename... Args>
requires CallableWith<Functor, Args...>
auto create_async_task(Functor&& func, Args&&... args) {
    return Future<InvokeResultT<Functor, Args...>, false, Functor, Args...>{ Forward<Functor>(func),
                                                                             Forward<Args>(args)... };
}
template <typename Functor, typename... Args>
requires CallableWith<Functor, Args...>
auto create_deferred_task(Functor&& func, Args&&... args) {
    return Future<InvokeResultT<Functor, Args...>, true, Functor, Args...>{ Forward<Functor>(func),
                                                                            Forward<Args>(args)... };
}
}    // namespace ARLib