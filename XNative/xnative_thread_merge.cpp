#define INCLUDED_FROM_OWN_CPP___
#include "xnative_thread_merge.h"

namespace ARLib {
#if defined(COMPILER_GCC) or defined(COMPILER_CLANG)
    Pair<ThreadT, bool> ThreadNative::create(ThreadRoutine routine, void* arg) {
        ThreadT p;
        auto res = pthread_create(&p, nullptr, routine, arg);
        return {p, res == 0};
    }
    int ThreadNative::detach(ThreadT thread) { return pthread_detach(thread); }
    int ThreadNative::join(ThreadT thread, RetVal* ret) { return pthread_join(thread, ret); }
    ThreadT ThreadNative::id() { return pthread_self(); }
    void ThreadNative::exit(RetVal val) { pthread_exit(val); }
    RetVal ThreadNative::retval_none() { return nullptr; }
    void ThreadNative::retval_destroy(RetVal val) {
        char* storage = static_cast<char*>(val);
        delete[] storage;
    }
    ThreadId ThreadNative::get_id(ThreadT thread) { return thread; }

    void ThreadNative::set_id(ThreadT& t1, ThreadId t2) { t1 = t2; }
    void ThreadNative::swap_id(ThreadT& t1, ThreadT& t2) {
        auto cp = t1;
        t1 = t2;
        t2 = cp;
    }

    Pair<MutexT, bool> MutexNative::init() {
        MutexT mtx{};
        auto state = pthread_mutex_init(&mtx, nullptr);
        return {mtx, state == 0};
    }
    Pair<MutexT, bool> MutexNative::init_try() {
        MutexT mtx{};
        PthreadMutexAttr attr{};
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::ERRORCHECK_NP));
        auto state = pthread_mutex_init(&mtx, &attr);
        return {mtx, state == 0};
    }
    Pair<MutexT, bool> MutexNative::init_timed() {
        MutexT mtx{};
        PthreadMutexAttr attr{};
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::TIMED_NP));
        auto state = pthread_mutex_init(&mtx, &attr);
        return {mtx, state == 0};
    }
    Pair<MutexT, bool> MutexNative::init_recursive() {
        MutexT mtx{};
        PthreadMutexAttr attr{};
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, static_cast<int>(PThreadMutex::RECURSIVE_NP));
        auto state = pthread_mutex_init(&mtx, &attr);
        return {mtx, state == 0};
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

    void MutexNative::destroy(MutexT& mutex) { pthread_mutex_destroy(&mutex); }
    bool MutexNative::lock(MutexT& mutex) { return pthread_mutex_lock(&mutex) == 0; }
    bool MutexNative::trylock(MutexT& mutex) { return pthread_mutex_trylock(&mutex) == 0; }
    bool MutexNative::timedlock(MutexT& mutex, MutexTimer timer) {
        return pthread_mutex_timedlock(&mutex, &timer) == 0;
    }
    bool MutexNative::unlock(MutexT& mutex) { return pthread_mutex_unlock(&mutex) == 0; }
#else
    Pair<ThreadT, bool> ThreadNative::create(ThreadRoutine routine, void* arg) {
        ThreadHandle t{};
        t._Hnd = reinterpret_cast<void*>(beginthreadex(nullptr, 0, routine, arg, 0, &t._Id));
        return {t, t._Hnd != nullptr};
    }
    int ThreadNative::detach(ThreadT thread) { return thread_detach(thread); }
    int ThreadNative::join(ThreadT thread, RetVal* ret) { return thread_join(thread, reinterpret_cast<int*>(ret)); }
    ThreadId ThreadNative::id() { return thread_id(); }
    void ThreadNative::exit(RetVal val) {
        cond_do_broadcast_at_thread_exit();
        endthreadex(val);
        unreachable
    }
    RetVal ThreadNative::retval_none() { return 0; }
    void ThreadNative::retval_destroy(RetVal) {}
    ThreadId ThreadNative::get_id(ThreadT thread) { return thread._Id; }
    void ThreadNative::set_id(ThreadT& t1, ThreadId t2) { t1._Id = t2; }
    void ThreadNative::swap_id(ThreadT& t1, ThreadT& t2) {
        auto cp = t1._Id;
        t1._Id = t2._Id;
        t2._Id = cp;
    }

    Pair<MutexT, bool> MutexNative::init() {
        MutexT mtx{};
        auto state = mutex_init(&mtx, MutexType::Plain);
        return {mtx, state == ThreadState::Success};
    }
    Pair<MutexT, bool> MutexNative::init_try() {
        MutexT mtx{};
        auto state = mutex_init(&mtx, MutexType::Try);
        return {mtx, state == ThreadState::Success};
    }
    Pair<MutexT, bool> MutexNative::init_timed() {
        MutexT mtx{};
        auto state = mutex_init(&mtx, MutexType::Timed);
        return {mtx, state == ThreadState::Success};
    }
    Pair<MutexT, bool> MutexNative::init_recursive() {
        MutexT mtx{};
        auto state = mutex_init(&mtx, MutexType::Recursive);
        return {mtx, state == ThreadState::Success};
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
            return static_cast<MutexType>(static_cast<UnderlyingTypeT<MutexType> >(a) |
                                          static_cast<UnderlyingTypeT<MutexType> >(b));
        };
        mutex_init(&mtx, combine(MutexType::Try, MutexType::Recursive));
        return mtx;
    }

    void MutexNative::destroy(MutexT& mutex) { mutex_destroy(mutex); }
    bool MutexNative::lock(MutexT& mutex) { return mutex_lock(mutex) == ThreadState::Success; }
    bool MutexNative::trylock(MutexT& mutex) { return mutex_trylock(mutex) == ThreadState::Success; }
    bool MutexNative::timedlock(MutexT& mutex, MutexTimer timer) {
        return mutex_timedlock(mutex, &timer) == ThreadState::Success;
    }
    bool MutexNative::unlock(MutexT& mutex) { return mutex_unlock(mutex) == ThreadState::Success; }

#endif
} // namespace ARLib