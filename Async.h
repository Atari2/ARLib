#pragma once
#include "Compat.h"
#include "Concepts.h"
#include "Threading.h"
#include "Printer.h"
namespace ARLib {
template <typename T, bool Deferred, typename Functor, typename... Args>
class Future {
    Thread m_executor_thread;
    Atomic<bool> m_result_ready{ false };
    UniquePtr<T> m_result;
    Mutex m_mutex;
    public:
    Future(Functor&& func, Args&&... args) {
        m_executor_thread = Thread{ [this, f = move(func)]<typename... LArgs>(LArgs&&... largs) {
                                       T intermediate = invoke(f, Forward<LArgs>(largs)...);
                                       {
                                           ScopedLock lock{ m_mutex };
                                           m_result = UniquePtr{ move(intermediate) };
                                       }
                                       m_result_ready.store(true);
                                   },
                                    Forward<Args>(args)... };
    }
    T wait() {
        if (!m_result_ready && m_executor_thread.joinable()) { m_executor_thread.join(); }
        return *m_result;
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