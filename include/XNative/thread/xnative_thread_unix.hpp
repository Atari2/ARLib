#pragma once
#ifndef DISABLE_THREADING

    #include "Compat.hpp"
    #include "Types.hpp"
    #ifdef UNIX_OR_MINGW
        #if not defined(THREADBASE_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
            #error "Don't include the XNative files directly. Use ThreadBase.h or Threading.h"
        #endif

        #ifdef ON_MINGW
            #define LOCKATTR_CONST
        #else
            #define LOCKATTR_CONST const
        #endif
namespace ARLib {
struct SchedParam {
    int sched_prio;
};
struct TimeSpec {
    long tv_sec;
    long tv_nsec;
};
        #define __LOCK_ALIGNMENT
        #define __ONCE_ALIGNMENT
namespace detail {
    typedef struct PthreadInternalList {
        struct PthreadInternalList* __prev;
        struct PthreadInternalList* __next;
    } PthreadList;
    struct PthreadMutexInternal {
        int __lock;
        unsigned int __count;
        int __owner;
        unsigned int __nusers;
        int __kind;
        short __spins;
        short __elision;
        PthreadList __list;
    };
    struct PthreadCondInternal {
        union {
            unsigned long long int __wseq;
            struct {
                unsigned int __low;
                unsigned int __high;
            } __wseq32;
        };
        union {
            unsigned long long int __g1_start;
            struct {
                unsigned int __low;
                unsigned int __high;
            } __g1_start32;
        };
        unsigned int __g_refs[2] __LOCK_ALIGNMENT;
        unsigned int __g_size[2];
        unsigned int __g1_orig_size;
        unsigned int __wrefs;
        unsigned int __g_signals[2];
    };
    struct PthreadRWLockArch {
        unsigned int __readers;
        unsigned int __writers;
        unsigned int __wrphase_futex;
        unsigned int __writers_futex;
        unsigned int __pad3;
        unsigned int __pad4;
        int __cur_writer;
        int __shared;
        signed char __rwelision;
        unsigned char __pad1[7];
        unsigned long int __pad2;
        unsigned int __flags;
    };
}    // namespace detail
using size_t                                = decltype(sizeof(void*));
constexpr auto SIZEOF_PTHREAD_MUTEX_T       = 40;
constexpr auto SIZEOF_PTHREAD_ATTR_T        = 56;
constexpr auto SIZEOF_PTHREAD_RWLOCK_T      = 56;
constexpr auto SIZEOF_PTHREAD_BARRIER_T     = 32;
constexpr auto SIZEOF_PTHREAD_MUTEXATTR_T   = 4;
constexpr auto SIZEOF_PTHREAD_COND_T        = 48;
constexpr auto SIZEOF_PTHREAD_CONDATTR_T    = 4;
constexpr auto SIZEOF_PTHREAD_RWLOCKATTR_T  = 8;
constexpr auto SIZEOF_PTHREAD_BARRIERATTR_T = 4;

        #ifdef ON_MINGW
typedef unsigned long long int Pthread;
        #else
typedef unsigned long int Pthread;
        #endif
typedef union {
    char __size[SIZEOF_PTHREAD_MUTEXATTR_T];
    int __align;
} PthreadMutexAttr;
/* Data structure for condition variable handling.  The structure of
       the attribute type is not exposed on purpose.  */
typedef union {
    char __size[SIZEOF_PTHREAD_CONDATTR_T];
    int __align;
} PthreadCondAttr;
typedef unsigned int PthreadKey;

typedef int __ONCE_ALIGNMENT PthreadOnce;
union PthreadAttr {
    char __size[SIZEOF_PTHREAD_ATTR_T];
    long int __align;
};
typedef union PthreadAttr PthreadAttr;
typedef union {
    struct detail::PthreadMutexInternal __data;
    char __size[SIZEOF_PTHREAD_MUTEX_T];
    long int __align;
} PthreadMutex;
typedef union {
    struct detail::PthreadCondInternal __data;
    char __size[SIZEOF_PTHREAD_COND_T];
    long long int __align;
} PthreadCond;
typedef union {
    struct detail::PthreadRWLockArch __data;
    char __size[SIZEOF_PTHREAD_RWLOCK_T];
    long int __align;
} PthreadRWLock;
typedef union {
    char __size[SIZEOF_PTHREAD_RWLOCKATTR_T];
    long int __align;
} PthreadRWLockAttr;
typedef volatile int PthreadSpinlock;
typedef union {
    char __size[SIZEOF_PTHREAD_BARRIER_T];
    long int __align;
} PthreadBarrier;
typedef union {
    char __size[SIZEOF_PTHREAD_BARRIERATTR_T];
    int __align;
} PthreadBarrierAttr;

enum class PThreadCreate { JOINABLE, DETACHED };
enum class PThreadMutex {
    TIMED_NP,
    RECURSIVE_NP,
    ERRORCHECK_NP,
    ADAPTIVE_NP,
    NORMAL     = TIMED_NP,
    RECURSIVE  = RECURSIVE_NP,
    ERRORCHECK = ERRORCHECK_NP,
    DEFAULT    = NORMAL,
    FAST_NP    = TIMED_NP
};

enum class PthreadMutexFlags { STALLED, STALLED_NP = STALLED, ROBUST, ROBUST_NP = ROBUST };

enum class PThreadPrio { NONE, INHERIT, PROTECT };

        #define __ARLIB_PTHREAD_MUTEX_INITIALIZER(__kind)                                                              \
            0, 0, 0, 0, __kind, 0, 0, { 0, 0 }

        #define ARLIB_PTHREAD_MUTEX_INITIALIZER                                                                        \
            {                                                                                                          \
                { __ARLIB_PTHREAD_MUTEX_INITIALIZER(static_cast<unsigned int>(PThreadMutex::TIMED_NP)) }               \
            }
        #define ARLIB_PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP                                                           \
            {                                                                                                          \
                { __ARLIB_PTHREAD_MUTEX_INITIALIZER(static_cast<unsigned int>(PThreadMutex::RECURSIVE_NP)) }           \
            }
        #define ARLIB_PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP                                                          \
            {                                                                                                          \
                { __ARLIB_PTHREAD_MUTEX_INITIALIZER(static_cast<unsigned int>(PThreadMutex::ERRORCHECK_NP)) }          \
            }
        #define ARLIB_PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP                                                            \
            {                                                                                                          \
                { __ARLIB_PTHREAD_MUTEX_INITIALIZER(static_cast<unsigned int>(PThreadMutex::ADAPTIVE_NP)) }            \
            }
enum class PThreadRWLock {
    PREFER_READER_NP,
    PREFER_WRITER_NP,
    PREFER_WRITER_NONRECURSIVE_NP,
    DEFAULT_NP = PREFER_READER_NP
};

        #define __ARLIB_PTHREAD_RWLOCK_ELISION_EXTRA                                                                   \
            0, { 0, 0, 0, 0, 0, 0, 0 }

        #define __ARLIB_PTHREAD_RWLOCK_INITIALIZER(__flags)                                                            \
            0, 0, 0, 0, 0, 0, 0, 0, __ARLIB_PTHREAD_RWLOCK_ELISION_EXTRA, 0, __flags

        #define ARLIB_PTHREAD_RWLOCK_INITIALIZER                                                                       \
            {                                                                                                          \
                { __ARLIB_PTHREAD_RWLOCK_INITIALIZER(static_cast<unsigned int>(PThreadRWLock::DEFAULT_NP)) }           \
            }
        #define ARLIB_PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP                                                \
            {                                                                                                          \
                {                                                                                                      \
                    __ARLIB_PTHREAD_RWLOCK_INITIALIZER(                                                                \
                    static_cast<unsigned int>(PThreadRWLock::PREFER_WRITER_NONRECURSIVE_NP)                            \
                    )                                                                                                  \
                }                                                                                                      \
            }
enum class PThreadSched { INHERIT, EXPLICIT };
enum class PThreadScope { SYSTEM, PROCESS };
enum class PThreadProcess { PRIVATE, SHARED };

        #define ARLIB_PTHREAD_COND_INITIALIZER                                                                         \
            {                                                                                                          \
                {                                                                                                      \
                    { 0 }, { 0 }, { 0, 0 }, { 0, 0 }, 0, 0, { 0, 0 }                                                   \
                }                                                                                                      \
            }
enum class PThreadCancel { ENABLE, DISABLE };
enum class PthreadCancelType { DEFERRED, ASYNCHRONOUS };
        #define ARLIB_PTHREAD_CANCELED (void*)(-1)

int pthread_sleep(int64_t millis);
int pthread_attr_destroy(PthreadAttr*);
int pthread_attr_getdetachstate(const PthreadAttr*, int*);
        #ifndef ON_MINGW
int pthread_attr_getguardsize(const PthreadAttr*, size_t*);
        #endif
int pthread_attr_getinheritsched(const PthreadAttr*, int*);
int pthread_attr_getschedparam(const PthreadAttr*, SchedParam*);
int pthread_attr_getschedpolicy(const PthreadAttr*, int*);
int pthread_attr_getscope(const PthreadAttr*, int*);
int pthread_attr_init(PthreadAttr*);
int pthread_attr_setdetachstate(PthreadAttr*, int);
        #ifndef ON_MINGW
int pthread_attr_setguardsize(PthreadAttr*, size_t);
        #endif
int pthread_attr_setinheritsched(PthreadAttr*, int);
int pthread_attr_setschedparam(PthreadAttr*, const SchedParam*);
int pthread_attr_setschedpolicy(PthreadAttr*, int);
int pthread_attr_setscope(PthreadAttr*, int);
int pthread_cancel(Pthread);
int pthread_cond_broadcast(PthreadCond*);
int pthread_cond_destroy(PthreadCond*);
int pthread_cond_init(PthreadCond*, const PthreadCondAttr*);
int pthread_cond_signal(PthreadCond*);
int pthread_cond_timedwait(PthreadCond*, PthreadMutex*, const TimeSpec*);
int pthread_cond_wait(PthreadCond*, PthreadMutex*);
int pthread_condattr_destroy(PthreadCondAttr*);
int pthread_condattr_getpshared(const PthreadCondAttr*, int*);
int pthread_condattr_init(PthreadCondAttr*);
int pthread_condattr_setpshared(PthreadCondAttr*, int);
int pthread_create(Pthread*, const PthreadAttr*, void* (*)(void*), void*);
int pthread_detach(Pthread);
int pthread_equal(Pthread, Pthread);
void pthread_exit(void*);
int pthread_getconcurrency(void);
int pthread_getschedparam(Pthread, int*, SchedParam*);
void* pthread_getspecific(PthreadKey);
int pthread_join(Pthread, void**);
int pthread_key_create(PthreadKey*, void (*)(void*));
int pthread_key_delete(PthreadKey);
int pthread_mutex_destroy(PthreadMutex*);
        #ifndef ON_MINGW
int pthread_mutex_getprioceiling(const PthreadMutex*, int*);
        #endif
int pthread_mutex_init(PthreadMutex*, const PthreadMutexAttr*);
int pthread_mutex_lock(PthreadMutex*);
        #ifndef ON_MINGW
int pthread_mutex_setprioceiling(PthreadMutex*, int, int*);
        #endif
int pthread_mutex_trylock(PthreadMutex*);
int pthread_mutex_timedlock(PthreadMutex*, const TimeSpec*);
int pthread_mutex_unlock(PthreadMutex*);
int pthread_mutexattr_destroy(PthreadMutexAttr*);
int pthread_mutexattr_getprioceiling(const PthreadMutexAttr*, int*);
int pthread_mutexattr_getprotocol(const PthreadMutexAttr*, int*);
int pthread_mutexattr_getpshared(const PthreadMutexAttr*, int*);
int pthread_mutexattr_gettype(const PthreadMutexAttr*, int*);
int pthread_mutexattr_init(PthreadMutexAttr*);
int pthread_mutexattr_setprioceiling(PthreadMutexAttr*, int);
int pthread_mutexattr_setprotocol(PthreadMutexAttr*, int);
int pthread_mutexattr_setpshared(PthreadMutexAttr*, int);
int pthread_mutexattr_settype(PthreadMutexAttr*, int);
int pthread_once(PthreadOnce*, void (*)(void));
int pthread_rwlock_destroy(PthreadRWLock*);
int pthread_rwlock_init(PthreadRWLock*, const PthreadRWLockAttr*);
int pthread_rwlock_rdlock(PthreadRWLock*);
int pthread_rwlock_tryrdlock(PthreadRWLock*);
int pthread_rwlock_trywrlock(PthreadRWLock*);
int pthread_rwlock_unlock(PthreadRWLock*);
int pthread_rwlock_wrlock(PthreadRWLock*);
int pthread_rwlockattr_destroy(PthreadRWLockAttr*);
int pthread_rwlockattr_getpshared(LOCKATTR_CONST PthreadRWLockAttr*, int*);
int pthread_rwlockattr_init(PthreadRWLockAttr*);
int pthread_rwlockattr_setpshared(PthreadRWLockAttr*, int);
Pthread pthread_self(void);
int pthread_setcancelstate(int, int*);
int pthread_setcanceltype(int, int*);
int pthread_setconcurrency(int);
int pthread_setschedparam(Pthread, int, const SchedParam*);
int pthread_setspecific(PthreadKey, const void*);
void pthread_testcancel(void);

}    // namespace ARLib
    #endif
#endif
