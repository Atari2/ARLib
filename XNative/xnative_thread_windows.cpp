#define INCLUDED_FROM_OWN_CPP___
#ifdef COMPILER_MSVC
#include "xnative_thread_windows.h"
#include "../TypeTraits.h"
#include <process.h>
#include <xthreads.h>

namespace ARLib {
    template <typename T, typename U>
    T _to_(U in) {
        return reinterpret_cast<T>(in);
    }

    template <typename T>
    requires IsEnumV<T> T _ec_(UnderlyingTypeT<T> in) { return static_cast<T>(in); }

    template <typename T>
    requires IsEnumV<T> UnderlyingTypeT<T> _uc_(T in) { return static_cast<UnderlyingTypeT<T>>(in); }

    static _Thrd_t ThreadToNative(ThreadHandle t) {
        _Thrd_t t_{t._Hnd, t._Id};
        return t_;
    }

    ThreadState __cdecl thread_detach(ThreadHandle thread) {
        return _ec_<ThreadState>(_Thrd_detach(ThreadToNative(thread)));
    }
    ThreadState __cdecl thread_join(ThreadHandle thread, int* ptr) {
        return _ec_<ThreadState>(_Thrd_join(ThreadToNative(thread), ptr));
    }
    void __cdecl thread_sleep(const XTime* time) { return _Thrd_sleep(_to_<const xtime*>(time)); }
    void __cdecl thread_yield() { _Thrd_yield(); }
    unsigned int __cdecl thread_hardware_concurrency() { return _Thrd_hardware_concurrency(); }
    ThreadId __cdecl thread_id() { return _Thrd_id(); }

    ThreadState __cdecl mutex_init(MutexHandle* mutex, MutexType init) {
        return _ec_<ThreadState>(_Mtx_init(_to_<_Mtx_t*>(mutex), _uc_(init)));
    }
    void __cdecl mutex_destroy(MutexHandle mutex) { _Mtx_destroy(_to_<_Mtx_t>(mutex)); }
    void __cdecl mutex_init_in_situ(MutexHandle mutex, MutexType init) {
        _Mtx_init_in_situ(_to_<_Mtx_t>(mutex), _uc_(init));
    }
    void __cdecl mutex_destroy_in_situ(MutexHandle mutex) { _Mtx_destroy_in_situ(_to_<_Mtx_t>(mutex)); }
    int __cdecl mutex_current_owns(MutexHandle mutex) { return _Mtx_current_owns(_to_<_Mtx_t>(mutex)); }
    ThreadState __cdecl mutex_lock(MutexHandle mutex) { return _ec_<ThreadState>(_Mtx_lock(_to_<_Mtx_t>(mutex))); }
    ThreadState __cdecl mutex_trylock(MutexHandle mutex) {
        return _ec_<ThreadState>(_Mtx_trylock(_to_<_Mtx_t>(mutex)));
    }
    ThreadState __cdecl mutex_timedlock(MutexHandle mutex, const XTime* time) {
        return _ec_<ThreadState>(_Mtx_timedlock(_to_<_Mtx_t>(mutex), _to_<const xtime*>(time)));
    }
    ThreadState __cdecl mutex_unlock(MutexHandle mutex) { return _ec_<ThreadState>(_Mtx_unlock(_to_<_Mtx_t>(mutex))); }

    void* __cdecl mutex_getconcrtcs(MutexHandle mutex) { return _Mtx_getconcrtcs(_to_<_Mtx_t>(mutex)); }
    void __cdecl mutex_clear_owner(MutexHandle mutex) { _Mtx_clear_owner(_to_<_Mtx_t>(mutex)); }
    void __cdecl mutex_reset_owner(MutexHandle mutex) { _Mtx_reset_owner(_to_<_Mtx_t>(mutex)); }

    void __cdecl sharedmutex_lock_exclusive(SharedMutex* smutex) { _Smtx_lock_exclusive(_to_<_Smtx_t*>(smutex)); }
    void __cdecl sharedmutex_lock_shared(SharedMutex* smutex) { _Smtx_lock_shared(_to_<_Smtx_t*>(smutex)); }
    int __cdecl sharedmutex_try_lock_exclusive(SharedMutex* smutex) {
        return _Smtx_try_lock_exclusive(_to_<_Smtx_t*>(smutex));
    }
    int __cdecl sharedmutex_try_lock_shared(SharedMutex* smutex) {
        return _Smtx_try_lock_shared(_to_<_Smtx_t*>(smutex));
    }
    void __cdecl sharedmutex_unlock_exclusive(SharedMutex* smutex) { _Smtx_unlock_exclusive(_to_<_Smtx_t*>(smutex)); }
    void __cdecl sharedmutex_unlock_shared(SharedMutex* smutex) { _Smtx_unlock_shared(_to_<_Smtx_t*>(smutex)); }

    int __cdecl cond_init(CondHandle* condition) { return _Cnd_init(_to_<_Cnd_t*>(condition)); }
    void __cdecl cond_destroy(CondHandle condition) { _Cnd_destroy(_to_<_Cnd_t>(condition)); }
    void __cdecl cond_init_in_situ(CondHandle condition) { _Cnd_init_in_situ(_to_<_Cnd_t>(condition)); }
    void __cdecl cond_destroy_in_situ(CondHandle condition) { _Cnd_destroy_in_situ(_to_<_Cnd_t>(condition)); }
    int __cdecl cond_wait(CondHandle condition, MutexHandle mutex) {
        return _Cnd_wait(_to_<_Cnd_t>(condition), _to_<_Mtx_t>(mutex));
    }
    int __cdecl cond_timedwait(CondHandle condition, MutexHandle mutex, const XTime* time) {
        return _Cnd_timedwait(_to_<_Cnd_t>(condition), _to_<_Mtx_t>(mutex), _to_<const xtime*>(time));
    }
    int __cdecl cond_broadcast(CondHandle condition) { return _Cnd_broadcast(_to_<_Cnd_t>(condition)); }
    int __cdecl cond_signal(CondHandle condition) { return _Cnd_signal(_to_<_Cnd_t>(condition)); }
    void __cdecl cond_register_at_thread_exit(CondHandle condition, MutexHandle mutex, int* ptr) {
        _Cnd_register_at_thread_exit(_to_<_Cnd_t>(condition), _to_<_Mtx_t>(mutex), ptr);
    }
    void __cdecl cond_unregister_at_thread_exit(MutexHandle mutex) {
        _Cnd_unregister_at_thread_exit(_to_<_Mtx_t>(mutex));
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