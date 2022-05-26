#define INCLUDED_FROM_OWN_CPP___
#include "xnative_thread_windows.h"
#ifdef WINDOWS
#include "../../Conversion.h"
#include "../../TypeTraits.h"
#include "../../cstring_compat.h"
#include <process.h>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <xthreads.h>

namespace ARLib {

    static_assert(sizeof(mutex_internal_imp_t) <= Mutex_internal_imp_size, "incorrect size of mutex implementation");
    static_assert(alignof(mutex_internal_imp_t) <= Mutex_internal_imp_alignment,
                  "incorrect alignment of mutex implementation");

    static_assert(sizeof(cnd_internal_imp_t) <= Cond_internal_imp_size, "incorrect condition variable implementation");
    static_assert(alignof(cnd_internal_imp_t) <= Cond_internal_imp_alignment,
                  "incorrect condition variable implementation");

    ThreadState __cdecl thread_detach(ThreadHandle thread) {
        return CloseHandle(thread._Hnd) == 0 ? ThreadState::Error : ThreadState::Success;
    }
    ThreadState __cdecl thread_join(ThreadHandle thread, int* ptr) {
        unsigned long res = 0;
        if (WaitForSingleObjectEx(thread._Hnd, INFINITE, FALSE) == WAIT_FAILED ||
            GetExitCodeThread(thread._Hnd, &res) == 0) {
            return ThreadState::Error;
        }
        if (ptr) { *ptr = static_cast<int>(res); }
        return CloseHandle(thread._Hnd) == 0 ? ThreadState::Error : ThreadState::Success;
    }
    void __cdecl thread_sleep(const XTime* time) {
        XTime now{};
        time_get(&now, TIME_UTC);
        do {
            Sleep(time_diff_to_millis2(time, &now));
            time_get(&now, TIME_UTC);
        } while (now.sec < time->sec || (now.sec == time->sec && now.nsec < time->nsec));
    }
    void __cdecl thread_yield() { SwitchToThread(); }
    unsigned int __cdecl thread_hardware_concurrency() {
        SYSTEM_INFO info;
        GetNativeSystemInfo(&info);
        return info.dwNumberOfProcessors;
    }
    ThreadId __cdecl thread_id() { return GetCurrentThreadId(); }

    ThreadState __cdecl mutex_init(MutexHandle* mtx, MutexType init) {
        *mtx = nullptr;

        auto* memory = new char[sizeof(mutex_internal_imp_t)];
        ARLib::memset(memory, 0, sizeof(mutex_internal_imp_t));
        MutexHandle mutex = new (memory) mutex_internal_imp_t;

        if (mutex == nullptr) { return ThreadState::Nomem; }

        mutex_init_in_situ(mutex, init);

        *mtx = mutex;
        return ThreadState::Success;
    }
    void __cdecl mutex_destroy(MutexHandle mutex) {
        if (mutex) {
            mutex_destroy_in_situ(mutex);
            delete[] cast<char*>(mutex);
        }
    }
    void __cdecl mutex_init_in_situ(MutexHandle mutex, MutexType init) {
        CriticalSection::Create(mutex->_get_cs());
        mutex->thread_id = -1;
        mutex->type = from_enum(init);
        mutex->count = 0;
    }
    void __cdecl mutex_destroy_in_situ(MutexHandle mutex) {
        HARD_ASSERT(mutex->count == 0, "Mutex destroyed while busy");
        mutex->_get_cs()->destroy();
    }
    int __cdecl mutex_current_owns(MutexHandle mutex) {
        return mutex->count != 0 && mutex->thread_id == static_cast<long>(GetCurrentThreadId());
    }

    static ThreadState mtx_do_lock(MutexHandle mutex, const XTime* target) {
        if (to_enum<MutexType>((mutex->type & ~from_enum(MutexType::Recursive))) == MutexType::Plain) {
            if (mutex->thread_id != static_cast<long>(GetCurrentThreadId())) {
                mutex->_get_cs()->lock();
                mutex->thread_id = static_cast<long>(GetCurrentThreadId());
            }
            ++mutex->count;

            return ThreadState::Success;
        } else {
            int res = WAIT_TIMEOUT;
            if (target == nullptr) {
                if (mutex->thread_id != static_cast<long>(GetCurrentThreadId())) { mutex->_get_cs()->lock(); }

                res = WAIT_OBJECT_0;

            } else if (target->sec < 0 || (target->sec == 0 && target->nsec <= 0)) {
                if (mutex->thread_id != static_cast<long>(GetCurrentThreadId())) {
                    if (mutex->_get_cs()->try_lock()) {
                        res = WAIT_OBJECT_0;
                    } else {
                        res = WAIT_TIMEOUT;
                    }
                } else {
                    res = WAIT_OBJECT_0;
                }

            } else {
                XTime now{};
                time_get(&now, TIME_UTC);
                while (now.sec < target->sec || (now.sec == target->sec && now.nsec < target->nsec)) {
                    if (mutex->thread_id == static_cast<long>(GetCurrentThreadId()) ||
                        mutex->_get_cs()->try_lock_for(time_diff_to_millis2(target, &now))) {
                        res = WAIT_OBJECT_0;
                        break;
                    } else {
                        res = WAIT_TIMEOUT;
                    }

                    time_get(&now, TIME_UTC);
                }
            }
            if (res == WAIT_OBJECT_0 || res == WAIT_ABANDONED) {
                if (1 < ++mutex->count) {
                    if (to_enum<MutexType>((mutex->type & from_enum(MutexType::Recursive))) != MutexType::Recursive) {
                        --mutex->count;
                        res = WAIT_TIMEOUT;
                    }
                } else {
                    mutex->thread_id = static_cast<long>(GetCurrentThreadId());
                }
            }

            switch (res) {
            case WAIT_OBJECT_0:
            case WAIT_ABANDONED:
                return ThreadState::Success;

            case WAIT_TIMEOUT:
                if (target == nullptr || (target->sec == 0 && target->nsec == 0)) {
                    return ThreadState::Busy;
                } else {
                    return ThreadState::Timeout;
                }

            default:
                return ThreadState::Error;
            }
        }
    }

    ThreadState __cdecl mutex_lock(MutexHandle mutex) { return mtx_do_lock(mutex, nullptr); }
    ThreadState __cdecl mutex_trylock(MutexHandle mutex) {
        XTime xt{};
        HARD_ASSERT((mutex->type & (from_enum(MutexType::Try) | from_enum(MutexType::Timed))) != 0,
                    "Trylock not supported by Mutex");
        xt.sec = 0;
        xt.nsec = 0;
        return mtx_do_lock(mutex, &xt);
    }
    ThreadState __cdecl mutex_timedlock(MutexHandle mutex, const XTime* time) {
        ThreadState res = ThreadState::Success;

        HARD_ASSERT((mutex->type & from_enum(MutexType::Timed)) != 0, "TimedLock not supported by Mutex");
        res = mtx_do_lock(mutex, time);
        return res == ThreadState::Busy ? ThreadState::Timeout : res;
    }
    ThreadState __cdecl mutex_unlock(MutexHandle mutex) {
        HARD_ASSERT(1 <= mutex->count && mutex->thread_id == static_cast<long>(GetCurrentThreadId()),
                    "Unlock of owned mutex");

        if (--mutex->count == 0) {
            mutex->thread_id = -1;
            mutex->_get_cs()->unlock();
        }
        return ThreadState::Success;
    }

    void* __cdecl mutex_getconcrtcs(MutexHandle mutex) { return mutex->_get_cs(); }
    void __cdecl mutex_clear_owner(MutexHandle mutex) {
        mutex->thread_id = -1;
        --mutex->count;
    }
    void __cdecl mutex_reset_owner(MutexHandle mutex) {
        mutex->thread_id = static_cast<long>(GetCurrentThreadId());
        ++mutex->count;
    }

    static_assert(sizeof(SharedMutex) == sizeof(SRWLOCK), "SharedMutex must be the same size as SRWLOCK.");
    static_assert(alignof(SharedMutex) == alignof(SRWLOCK), "SharedMutex must be the same alignment as SRWLOCK.");

    void __cdecl sharedmutex_lock_exclusive(SharedMutex* smutex) { AcquireSRWLockExclusive(cast<PSRWLOCK>(smutex)); }
    void __cdecl sharedmutex_lock_shared(SharedMutex* smutex) { AcquireSRWLockShared(cast<PSRWLOCK>(smutex)); }
    int __cdecl sharedmutex_try_lock_exclusive(SharedMutex* smutex) {
        return TryAcquireSRWLockExclusive(cast<PSRWLOCK>(smutex));
    }
    int __cdecl sharedmutex_try_lock_shared(SharedMutex* smutex) {
        return TryAcquireSRWLockShared(cast<PSRWLOCK>(smutex));
    }
    void __cdecl sharedmutex_unlock_exclusive(SharedMutex* smutex) { ReleaseSRWLockExclusive(cast<PSRWLOCK>(smutex)); }
    void __cdecl sharedmutex_unlock_shared(SharedMutex* smutex) { ReleaseSRWLockShared(cast<PSRWLOCK>(smutex)); }

    ThreadState __cdecl cond_init(CondHandle* condition) {
        *condition = nullptr;

        char* memory = new char[sizeof(cnd_internal_imp_t)];
        ARLib::memset(memory, 0, sizeof(cnd_internal_imp_t));
        const CondHandle cond = new (memory) cnd_internal_imp_t;
        if (cond == nullptr) {
            return ThreadState::Nomem;
        }

        cond_init_in_situ(cond);
        *condition = cond;
        return ThreadState::Success;
    }
    void __cdecl cond_destroy(CondHandle condition) {
        if (condition) {
            cond_destroy_in_situ(condition);
            delete[] cast<char*>(condition);
        }
    }
    void __cdecl cond_init_in_situ(CondHandle condition) { ConditionVariable::Create(condition->_get_cv()); }
    void __cdecl cond_destroy_in_situ(CondHandle condition) { condition->_get_cv()->destroy(); }
    ThreadState __cdecl cond_wait(CondHandle condition, MutexHandle mutex) {
        const auto cs = static_cast<CriticalSection::Interface*>(mutex_getconcrtcs(mutex));
        mutex_clear_owner(mutex);
        condition->_get_cv()->wait(cs);
        mutex_reset_owner(mutex);
        return ThreadState::Success;
    }
    ThreadState __cdecl cond_timedwait(CondHandle condition, MutexHandle mutex, const XTime* target) {
        ThreadState res = ThreadState::Success;
        const auto cs = static_cast<CriticalSection::Interface*>(mutex_getconcrtcs(mutex));
        if (target == nullptr) {
            mutex_clear_owner(mutex);
            condition->_get_cv()->wait(cs);
            mutex_reset_owner(mutex);
        } else {
            XTime now;
            time_get(&now, TIME_UTC);
            mutex_clear_owner(mutex);
            if (!condition->_get_cv()->wait_for(cs, time_diff_to_millis2(target, &now))) {
                time_get(&now, TIME_UTC);
                if (time_diff_to_millis2(target, &now) == 0) { res = ThreadState::Timeout; }
            }
            mutex_reset_owner(mutex);
        }
        return res;
    }
    ThreadState __cdecl cond_broadcast(CondHandle condition) {
        condition->_get_cv()->notify_one();
        return ThreadState::Success;
    }
    ThreadState __cdecl cond_signal(CondHandle condition) {
        condition->_get_cv()->notify_all();
        return ThreadState::Success;
    }

    void __cdecl cond_do_broadcast_at_thread_exit() { _Cnd_do_broadcast_at_thread_exit(); }

    uintptr_t __cdecl beginthread(beginthread_proc_type _StartAddress, unsigned _StackSize, void* _ArgList) {
        return _beginthread(_StartAddress, _StackSize, _ArgList);
    }

    void __cdecl endthread(void) { _endthread(); }
    uintptr_t __cdecl beginthreadex(void* _Security, unsigned _StackSize, beginthreadex_proc_type _StartAddress,
                                    void* _ArgList, unsigned _InitFlag, unsigned* _ThrdAddr) {
        return _beginthreadex(_Security, _StackSize, _StartAddress, _ArgList, _InitFlag, _ThrdAddr);
    }

    void __cdecl endthreadex(unsigned _ReturnCode) { _endthreadex(_ReturnCode); }
} // namespace ARLib
#endif