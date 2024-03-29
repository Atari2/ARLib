#ifndef INCLUDED_FROM_OWN_CPP___
    #define INCLUDED_FROM_OWN_CPP___
#endif
#include "XNative/thread/xnative_thread_merge.hpp"
#include "Threading.hpp"
#include "Chrono.hpp"
namespace ARLib {
#ifdef UNIX_OR_MINGW
Pair<ThreadT, bool> ThreadNative::create(ThreadRoutine routine, void* arg) {
    ThreadT p;
    auto res = pthread_create(&p, nullptr, routine, arg);
    return { p, res == 0 };
}
int ThreadNative::detach(ThreadT thread) {
    return pthread_detach(thread);
}
int ThreadNative::join(ThreadT thread, RetVal* ret) {
    return pthread_join(thread, ret);
}
ThreadT ThreadNative::id() {
    return pthread_self();
}
void ThreadNative::exit(RetVal val) {
    pthread_exit(val);
}
RetVal ThreadNative::retval_none() {
    return nullptr;
}
void ThreadNative::retval_destroy(RetVal val) {
    char* storage = static_cast<char*>(val);
    delete[] storage;
}
ThreadId ThreadNative::get_id(ThreadT thread) {
    return thread;
}
void ThreadNative::set_id(ThreadT& t1, ThreadId t2) {
    t1 = t2;
}
void ThreadNative::swap(ThreadT& t1, ThreadT& t2) {
    auto cp = t1;
    t1      = t2;
    t2      = cp;
}
void ThreadNative::sleep(Micros microseconds) {
    pthread_sleep(microseconds.value);
}
Pair<MutexT, bool> MutexNative::init() {
    MutexT mtx{};
    auto state = pthread_mutex_init(&mtx, nullptr);
    return { mtx, state == 0 };
}
Pair<MutexT, bool> MutexNative::init_try() {
    MutexT mtx{};
    PthreadMutexAttr attr{};
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::ERRORCHECK_NP));
    auto state = pthread_mutex_init(&mtx, &attr);
    return { mtx, state == 0 };
}
Pair<MutexT, bool> MutexNative::init_timed() {
    MutexT mtx{};
    PthreadMutexAttr attr{};
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::TIMED_NP));
    auto state = pthread_mutex_init(&mtx, &attr);
    return { mtx, state == 0 };
}
Pair<MutexT, bool> MutexNative::init_recursive() {
    MutexT mtx{};
    PthreadMutexAttr attr{};
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::RECURSIVE_NP));
    auto state = pthread_mutex_init(&mtx, &attr);
    return { mtx, state == 0 };
}
MutexT MutexNative::init_noret() {
    MutexT mtx{};
    pthread_mutex_init(&mtx, nullptr);
    return mtx;
}
MutexT MutexNative::init_try_noret() {
    MutexT mtx{};
    PthreadMutexAttr attr{};
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::ERRORCHECK_NP));
    pthread_mutex_init(&mtx, &attr);
    return mtx;
}
MutexT MutexNative::init_timed_noret() {
    MutexT mtx{};
    PthreadMutexAttr attr{};
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::TIMED_NP));
    pthread_mutex_init(&mtx, &attr);
    return mtx;
}
MutexT MutexNative::init_recursive_noret() {
    MutexT mtx{};
    PthreadMutexAttr attr{};
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::RECURSIVE_NP));
    pthread_mutex_init(&mtx, &attr);
    return mtx;
}
void MutexNative::destroy(MutexT& mutex) {
    pthread_mutex_destroy(&mutex);
}
bool MutexNative::lock(MutexT& mutex) {
    return pthread_mutex_lock(&mutex) == 0;
}
bool MutexNative::trylock(MutexT& mutex) {
    return pthread_mutex_trylock(&mutex) == 0;
}
bool MutexNative::timedlock(MutexT& mutex, MutexTimer timer) {
    return pthread_mutex_timedlock(&mutex, &timer) == 0;
}
bool MutexNative::unlock(MutexT& mutex) {
    return pthread_mutex_unlock(&mutex) == 0;
}
Pair<ConditionVariableT, bool> ConditionVariableNative::init() {
    ConditionVariableT cv{};
    pthread_cond_init(&cv, nullptr);
    return { cv, true };
}
ConditionVariableT ConditionVariableNative::init_noret() {
    ConditionVariableT cv{};
    pthread_cond_init(&cv, nullptr);
    return cv;
}
void ConditionVariableNative::destroy(ConditionVariableT& cv) {
    pthread_cond_destroy(&cv);
}
void ConditionVariableNative::notify_one(ConditionVariableT& cv) {
    pthread_cond_signal(&cv);
}
void ConditionVariableNative::notify_all(ConditionVariableT& cv) {
    pthread_cond_broadcast(&cv);
}
void ConditionVariableNative::wait(ConditionVariableT& cv, UniqueLock<Mutex>* lock) {
    pthread_cond_wait(&cv, lock->mutex()->native_handle());
}
CVStatus ConditionVariableNative::wait_for(ConditionVariableT& cv, UniqueLock<Mutex>* lock, Duration ns) {
    Nanos raw          = ns.raw_value();
    TimeSpecC spec     = time_get();
    constexpr auto den = 1'000'000'000L;
    spec.tv_sec += raw.value / den;
    spec.tv_nsec += raw.value % den;
    int ret = pthread_cond_timedwait(&cv, lock->mutex()->native_handle(), cast<TimeSpec*>(&spec));
    return ret == 0 ? CVStatus::NoTimeout : CVStatus::Timeout;
}
CVStatus ConditionVariableNative::wait_until(ConditionVariableT& cv, UniqueLock<Mutex>* lock, Instant ns) {
    TimeSpec spec{};
    Nanos raw          = ns.raw_value();
    constexpr auto den = 1'000'000'000L;
    spec.tv_sec        = raw.value / den;
    spec.tv_nsec       = raw.value % den;
    int ret            = pthread_cond_timedwait(&cv, lock->mutex()->native_handle(), &spec);
    return ret == 0 ? CVStatus::NoTimeout : CVStatus::Timeout;
}
#else
Pair<ThreadT, bool> ThreadNative::create(ThreadRoutine routine, void* arg) {
    ThreadHandle t{};
    t._Hnd = reinterpret_cast<void*>(beginthreadex(nullptr, 0, routine, arg, 0, &t._Id));
    return { t, t._Hnd != nullptr };
}
ThreadState ThreadNative::detach(ThreadT thread) {
    return thread_detach(thread);
}
ThreadState ThreadNative::join(ThreadT thread, RetVal* ret) {
    return thread_join(thread, reinterpret_cast<int*>(ret));
}
ThreadId ThreadNative::id() {
    return thread_id();
}
void ThreadNative::exit(RetVal val) {
    cond_do_broadcast_at_thread_exit();
    endthreadex(val);
    arlib_unreachable
}
RetVal ThreadNative::retval_none() {
    return 0;
}
void ThreadNative::retval_destroy(RetVal) {}
ThreadId ThreadNative::get_id(ThreadT thread) {
    return thread._Id;
}
void ThreadNative::set_id(ThreadT& t1, ThreadId t2) {
    t1._Id = t2;
}
void ThreadNative::swap(ThreadT& t1, ThreadT& t2) {
    auto hdl = t1._Hnd;
    auto cp  = t1._Id;
    t1._Id   = t2._Id;
    t1._Hnd  = t2._Hnd;
    t2._Id   = cp;
    t2._Hnd  = hdl;
}
void ThreadNative::sleep(Micros microseconds) {
    thread_sleep_microseconds(microseconds.value);
}
Pair<MutexT, bool> MutexNative::init() {
    MutexT mtx{};
    auto state = mutex_init(&mtx, MutexType::Plain);
    return { mtx, state == ThreadState::Success };
}
Pair<MutexT, bool> MutexNative::init_try() {
    MutexT mtx{};
    auto state = mutex_init(&mtx, MutexType::Try);
    return { mtx, state == ThreadState::Success };
}
Pair<MutexT, bool> MutexNative::init_timed() {
    MutexT mtx{};
    auto state = mutex_init(&mtx, MutexType::Timed);
    return { mtx, state == ThreadState::Success };
}
Pair<MutexT, bool> MutexNative::init_recursive() {
    MutexT mtx{};
    auto state = mutex_init(&mtx, MutexType::Recursive);
    return { mtx, state == ThreadState::Success };
}
MutexT MutexNative::init_noret() {
    MutexT mtx{};
    mutex_init(&mtx, MutexType::Plain);
    return mtx;
}
MutexT MutexNative::init_try_noret() {
    MutexT mtx{};
    mutex_init(&mtx, MutexType::Try);
    return mtx;
}
MutexT MutexNative::init_timed_noret() {
    MutexT mtx{};
    mutex_init(&mtx, MutexType::Timed);
    return mtx;
}
MutexT MutexNative::init_recursive_noret() {
    MutexT mtx{};
    constexpr auto combine = [](MutexType a, MutexType b) {
        return static_cast<MutexType>(
        static_cast<UnderlyingTypeT<MutexType>>(a) | static_cast<UnderlyingTypeT<MutexType>>(b)
        );
    };
    mutex_init(&mtx, combine(MutexType::Try, MutexType::Recursive));
    return mtx;
}
void MutexNative::destroy(MutexT& mutex) {
    mutex_destroy(mutex);
}
bool MutexNative::lock(MutexT& mutex) {
    return mutex_lock(mutex) == ThreadState::Success;
}
bool MutexNative::trylock(MutexT& mutex) {
    return mutex_trylock(mutex) == ThreadState::Success;
}
bool MutexNative::timedlock(MutexT& mutex, MutexTimer timer) {
    return mutex_timedlock(mutex, &timer) == ThreadState::Success;
}
bool MutexNative::unlock(MutexT& mutex) {
    return mutex_unlock(mutex) == ThreadState::Success;
}
Pair<ConditionVariableT, bool> ConditionVariableNative::init() {
    ConditionVariableT cv{};
    cond_init_in_situ(&cv);
    return { cv, true };
}
ConditionVariableT ConditionVariableNative::init_noret() {
    ConditionVariableT cv{};
    cond_init_in_situ(&cv);
    return cv;
}
void ConditionVariableNative::destroy(ConditionVariableT& cv) {
    cond_destroy_in_situ(&cv);
}
void ConditionVariableNative::notify_one(ConditionVariableT& cv) {
    cond_signal(&cv);
}
void ConditionVariableNative::notify_all(ConditionVariableT& cv) {
    cond_broadcast(&cv);
}
void ConditionVariableNative::wait(ConditionVariableT& cv, UniqueLock<Mutex>* lock) {
    cond_wait(&cv, *lock->mutex()->native_handle());
}
CVStatus ConditionVariableNative::wait_for(ConditionVariableT& cv, UniqueLock<Mutex>* lock, Duration ns) {
    Nanos raw = ns.raw_value();
    XTime spec{};
    spec.sec  = raw.value / 1'000'000'000;
    spec.nsec = raw.value % 1'000'000'000;
    return cond_rel_timedwait(&cv, *lock->mutex()->native_handle(), &spec) == ThreadState::Timeout ?
           CVStatus::Timeout :
           CVStatus::NoTimeout;
}
CVStatus ConditionVariableNative::wait_until(ConditionVariableT& cv, UniqueLock<Mutex>* lock, Instant ns) {
    Nanos raw = ns.raw_value();
    XTime spec{};
    spec.sec  = raw.value / 1'000'000'000;
    spec.nsec = raw.value % 1'000'000'000;
    return cond_timedwait(&cv, *lock->mutex()->native_handle(), &spec) == ThreadState::Timeout ? CVStatus::Timeout :
                                                                                                 CVStatus::NoTimeout;
}
#endif
}    // namespace ARLib
