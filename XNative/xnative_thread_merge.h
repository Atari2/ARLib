#pragma once
#ifndef DISABLE_THREADING
#ifdef ON_LINUX
#include "xnative_thread_unix.h"
#else
#include "xnative_thread_windows.h"
#endif
#include "../Pair.h"

#if not defined(THREADBASE_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
#error "Don't include the XNative files directly. Use ThreadBase.h or Threading.h"
#endif

namespace ARLib {
    [[noreturn]] void inline arlib_terminate() { abort_arlib(); unreachable }
#ifdef ON_LINUX
    /* THREADS */
    using ThreadT = Pthread;
    using ThreadId = Pthread;
    using ThreadRoutine = void* (*)(void*);
    using RetVal = void*;
#define TEMPLATE  template <typename Tp, typename... Args>
#define ARGS_DECL Args... args

    /* MUTEX */
    using MutexT = PthreadMutex;
    using MutexTimer = TimeSpec;
    using MutexAttributes = PthreadMutexAttr;
#else
    /* THREADS */
    using ThreadT = ThreadHandle;
    using ThreadId = ThreadId;
    using ThreadRoutine = beginthreadex_proc_type;
    using RetVal = unsigned;
#define TEMPLATE  template <typename, typename...>
#define ARGS_DECL RetVal ret
    
    /* MUTEX */
    using MutexT = Mutex;
    using MutexTimer = XTime;
    using MutexAttributes = MutexType;
#endif
    class ThreadNative {
        public:
        static Pair<ThreadT, bool> create(ThreadRoutine, void*);
        static int detach(ThreadT);
        static int join(ThreadT, RetVal*);
        static ThreadId id();
        static void exit(RetVal);
        static RetVal retval_none();
        static void retval_destroy(RetVal);
        static ThreadId get_id(ThreadT);
        static void set_id(ThreadT&, ThreadId);
        static void swap_id(ThreadT&, ThreadT&);
        TEMPLATE
        static RetVal retval_create(ARGS_DECL) {
#ifdef ON_LINUX
            if constexpr (alignof(Tp) < alignof(char*)) {
                char* storage = new char[sizeof(Tp)];
                new (storage) Tp{args...};
                return storage;
            } else {
                alignas(Tp) char* storage = new char[sizeof(Tp)];
                new (storage) Tp{args...};
                return storage;
            }
#else
            return ret;
#endif
        }
        template <typename Tp>
        static decltype(auto) retval_read(RetVal val) {
#ifdef ON_LINUX
            return static_cast<Tp&>(*static_cast<Tp*>(val));
#else
            return val;
#endif
        }
    };

    class MutexNative {
        static Pair<MutexT, bool> init();
        static Pair<MutexT, bool> init_try();
        static Pair<MutexT, bool> init_timed();
        static Pair<MutexT, bool> init_recursive();
        static void destroy(MutexT);
        static bool lock(MutexT);
        static bool trylock(MutexT);
        static bool timedlock(MutexT, MutexTimer);
        static bool unlock(MutexT);
    };
} // namespace ARLib
#endif