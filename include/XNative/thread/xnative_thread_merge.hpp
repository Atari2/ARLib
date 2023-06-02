#pragma once
#ifndef DISABLE_THREADING

    #include "Compat.hpp"
    #ifdef UNIX_OR_MINGW
        #include "XNative/thread/xnative_thread_unix.hpp"
    #else
        #include "XNative/thread/xnative_thread_windows.hpp"
    #endif
    #include "Pair.hpp"
    #include "Chrono.hpp"

    #if not defined(THREADBASE_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
        #error "Don't include the XNative files directly. Use ThreadBase.h or Threading.h"
    #endif
namespace ARLib {
[[noreturn]] inline void arlib_terminate() {
    abort_arlib();
    arlib_unreachable
}
    #ifdef UNIX_OR_MINGW
/* THREADS */
using ThreadT       = Pthread;
using ThreadId      = Pthread;
using ThreadRoutine = void* (*)(void*);
using RetVal        = void*;
using ThreadState   = int;
        #define TEMPLATE  template <typename Tp, typename... Args>
        #define ARGS_DECL Args... args

/* MUTEX */
using MutexT          = PthreadMutex;
using MutexTimer      = TimeSpec;
using MutexAttributes = PthreadMutexAttr;

/* CONDITION_VARIABLE */
using ConditionVariableT = PthreadCond;
    #else
/* THREADS */
using ThreadT       = ThreadHandle;
using ThreadId      = ThreadId;
using ThreadRoutine = beginthreadex_proc_type;
using RetVal        = unsigned;
        #define TEMPLATE  template <typename, typename...>
        #define ARGS_DECL RetVal ret

/* MUTEX */
using MutexT          = MutexHandle;
using MutexTimer      = XTime;
using MutexAttributes = MutexType;
/* CONDITION_VARIABLE */
using ConditionVariableT = CondInternalImplType;

    #endif
class ThreadNative {
    public:
    static Pair<ThreadT, bool> create(ThreadRoutine, void*);
    static ThreadState detach(ThreadT);
    static ThreadState join(ThreadT, RetVal*);
    static ThreadId id();
    static void exit(RetVal);
    static RetVal retval_none();
    static void retval_destroy(RetVal);
    static ThreadId get_id(ThreadT);
    static void set_id(ThreadT&, ThreadId);
    static void swap(ThreadT&, ThreadT&);
    static void sleep(int64_t millis);
    TEMPLATE
    static RetVal retval_create(ARGS_DECL) {
    #ifdef UNIX_OR_MINGW
        if constexpr (alignof(Tp) < alignof(char*)) {
            char* storage = new char[sizeof(Tp)];
            new (storage) Tp{ args... };
            return storage;
        } else {
            alignas(Tp) char* storage = new char[sizeof(Tp)];
            new (storage) Tp{ args... };
            return storage;
        }
    #else
        return ret;
    #endif
    }
    template <typename Tp>
    static decltype(auto) retval_read(RetVal val) {
    #ifdef UNIX_OR_MINGW
        return static_cast<Tp&>(*static_cast<Tp*>(val));
    #else
        return val;
    #endif
    }
};
class MutexNative {
    public:
    static Pair<MutexT, bool> init();
    static Pair<MutexT, bool> init_try();
    static Pair<MutexT, bool> init_timed();
    static Pair<MutexT, bool> init_recursive();
    static MutexT init_noret();
    static MutexT init_try_noret();
    static MutexT init_timed_noret();
    static MutexT init_recursive_noret();
    static void destroy(MutexT&);
    static bool lock(MutexT&);
    static bool trylock(MutexT&);
    static bool timedlock(MutexT&, MutexTimer);
    static bool unlock(MutexT&);
};

enum class CVStatus { Timeout, NoTimeout };

template <typename M>
class UniqueLock;
class Mutex;
class ConditionVariableNative {
    public:
    static Pair<ConditionVariableT, bool> init();
    static ConditionVariableT init_noret();
    static void destroy(ConditionVariableT&);
    static void notify_one(ConditionVariableT&);
    static void notify_all(ConditionVariableT&);

    static void wait(ConditionVariableT&, UniqueLock<Mutex>*);
    template <class Predicate>
    static void wait(ConditionVariableT& cv, UniqueLock<Mutex>* lock, Predicate stop_waiting) {
        while (!stop_waiting()) { ConditionVariableNative::wait(cv, lock); }
    }
    static CVStatus wait_for(ConditionVariableT&, UniqueLock<Mutex>*, Duration ns);
    template <class Predicate>
    static bool wait_for(ConditionVariableT& cv, UniqueLock<Mutex>* lock, Duration rel, Predicate stop_waiting) {
        while (!stop_waiting()) {
            if (ConditionVariableNative::wait_for(cv, lock, rel) == CVStatus::Timeout) { return stop_waiting(); }
        }
        return true;
    }
    static CVStatus wait_until(ConditionVariableT&, UniqueLock<Mutex>*, Instant ns);
    template <class Predicate>
    static bool wait_until(ConditionVariableT& cv, UniqueLock<Mutex>* lock, Instant ns, Predicate stop_waiting) {
        while (!stop_waiting()) {
            if (ConditionVariableNative::wait_until(cv, lock, ns) == CVStatus::Timeout) { return stop_waiting(); }
        }
        return true;
    }
};
}    // namespace ARLib
#endif
