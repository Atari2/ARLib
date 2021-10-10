#define INCLUDED_FROM_OWN_CPP___
#include "xnative_thread_windows.h"
#ifdef WINDOWS
#include "../../TypeTraits.h"
#include "../../Conversion.h"
#include <process.h>
#include <xthreads.h>

namespace ARLib {
    static _Thrd_t ThreadToNative(ThreadHandle t) {
        _Thrd_t t_{t._Hnd, t._Id};
        return t_;
    }

    ThreadState __cdecl thread_detach(ThreadHandle thread) {
        return to_enum<ThreadState>(_Thrd_detach(ThreadToNative(thread)));
    }
    ThreadState __cdecl thread_join(ThreadHandle thread, int* ptr) {
        return to_enum<ThreadState>(_Thrd_join(ThreadToNative(thread), ptr));
    }
    void __cdecl thread_sleep(const XTime* time) { return _Thrd_sleep(cast<const xtime*>(time)); }
    void __cdecl thread_yield() { _Thrd_yield(); }
    unsigned int __cdecl thread_hardware_concurrency() { return _Thrd_hardware_concurrency(); }
    ThreadId __cdecl thread_id() { return _Thrd_id(); }

    ThreadState __cdecl mutex_init(MutexHandle* mutex, MutexType init) {
        return to_enum<ThreadState>(_Mtx_init(cast<_Mtx_t*>(mutex), from_enum(init)));
    }
    void __cdecl mutex_destroy(MutexHandle mutex) { _Mtx_destroy(cast<_Mtx_t>(mutex)); }
    void __cdecl mutex_init_in_situ(MutexHandle mutex, MutexType init) {
        _Mtx_init_in_situ(cast<_Mtx_t>(mutex), from_enum(init));
    }
    void __cdecl mutex_destroy_in_situ(MutexHandle mutex) { _Mtx_destroy_in_situ(cast<_Mtx_t>(mutex)); }
    int __cdecl mutex_current_owns(MutexHandle mutex) { return _Mtx_current_owns(cast<_Mtx_t>(mutex)); }
    ThreadState __cdecl mutex_lock(MutexHandle mutex) { return to_enum<ThreadState>(_Mtx_lock(cast<_Mtx_t>(mutex))); }
    ThreadState __cdecl mutex_trylock(MutexHandle mutex) {
        return to_enum<ThreadState>(_Mtx_trylock(cast<_Mtx_t>(mutex)));
    }
    ThreadState __cdecl mutex_timedlock(MutexHandle mutex, const XTime* time) {
        return to_enum<ThreadState>(_Mtx_timedlock(cast<_Mtx_t>(mutex), cast<const xtime*>(time)));
    }
    ThreadState __cdecl mutex_unlock(MutexHandle mutex) { return to_enum<ThreadState>(_Mtx_unlock(cast<_Mtx_t>(mutex))); }

    void* __cdecl mutex_getconcrtcs(MutexHandle mutex) { return _Mtx_getconcrtcs(cast<_Mtx_t>(mutex)); }
    void __cdecl mutex_clear_owner(MutexHandle mutex) { _Mtx_clear_owner(cast<_Mtx_t>(mutex)); }
    void __cdecl mutex_reset_owner(MutexHandle mutex) { _Mtx_reset_owner(cast<_Mtx_t>(mutex)); }

    void __cdecl sharedmutex_lock_exclusive(SharedMutex* smutex) { _Smtx_lock_exclusive(cast<_Smtx_t*>(smutex)); }
    void __cdecl sharedmutex_lock_shared(SharedMutex* smutex) { _Smtx_lock_shared(cast<_Smtx_t*>(smutex)); }
    int __cdecl sharedmutex_try_lock_exclusive(SharedMutex* smutex) {
        return _Smtx_try_lock_exclusive(cast<_Smtx_t*>(smutex));
    }
    int __cdecl sharedmutex_try_lock_shared(SharedMutex* smutex) {
        return _Smtx_try_lock_shared(cast<_Smtx_t*>(smutex));
    }
    void __cdecl sharedmutex_unlock_exclusive(SharedMutex* smutex) { _Smtx_unlock_exclusive(cast<_Smtx_t*>(smutex)); }
    void __cdecl sharedmutex_unlock_shared(SharedMutex* smutex) { _Smtx_unlock_shared(cast<_Smtx_t*>(smutex)); }

    int __cdecl cond_init(CondHandle* condition) { return _Cnd_init(cast<_Cnd_t*>(condition)); }
    void __cdecl cond_destroy(CondHandle condition) { _Cnd_destroy(cast<_Cnd_t>(condition)); }
    void __cdecl cond_init_in_situ(CondHandle condition) { _Cnd_init_in_situ(cast<_Cnd_t>(condition)); }
    void __cdecl cond_destroy_in_situ(CondHandle condition) { _Cnd_destroy_in_situ(cast<_Cnd_t>(condition)); }
    int __cdecl cond_wait(CondHandle condition, MutexHandle mutex) {
        return _Cnd_wait(cast<_Cnd_t>(condition), cast<_Mtx_t>(mutex));
    }
    int __cdecl cond_timedwait(CondHandle condition, MutexHandle mutex, const XTime* time) {
        return _Cnd_timedwait(cast<_Cnd_t>(condition), cast<_Mtx_t>(mutex), cast<const xtime*>(time));
    }
    int __cdecl cond_broadcast(CondHandle condition) { return _Cnd_broadcast(cast<_Cnd_t>(condition)); }
    int __cdecl cond_signal(CondHandle condition) { return _Cnd_signal(cast<_Cnd_t>(condition)); }
    void __cdecl cond_register_at_thread_exit(CondHandle condition, MutexHandle mutex, int* ptr) {
        _Cnd_register_at_thread_exit(cast<_Cnd_t>(condition), cast<_Mtx_t>(mutex), ptr);
    }
    void __cdecl cond_unregister_at_thread_exit(MutexHandle mutex) {
        _Cnd_unregister_at_thread_exit(cast<_Mtx_t>(mutex));
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