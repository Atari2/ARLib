#pragma once
#ifndef DISABLE_THREADING
#include "../../Types.h"
#include "../../Windows/win_native_structs.h"
#define INCLUDED_FROM_XNATIVE_THREADS__
#include "../chrono/xnative_chrono_windows.h"

#ifdef WINDOWS
#if not defined(THREADBASE_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
#error "Don't include the XNative files directly. Use ThreadBase.h or Threading.h"
#endif

namespace ARLib {

    using ThreadId = ThreadIdImplType;
    using ThreadHandle = ThreadImplType;
    using XTime = XTimeC;

    constexpr size_t Mutex_internal_imp_size = 80;
    constexpr size_t Mutex_internal_imp_alignment = 8;
    constexpr size_t Cond_internal_imp_size = 72;
    constexpr size_t Cond_internal_imp_alignment = 8;

    using MutexHandle = mutex_internal_imp_t*;

    using CondHandle = cnd_internal_imp_t*;

    using SharedMutex = void*;

    enum class ThreadState { Success, Nomem, Timeout, Busy, Error };
    enum class MutexType { Plain = 0x01, Try = 0x02, Timed = 0x04, Recursive = 0x100 };

    ThreadState __cdecl thread_detach(ThreadHandle);
    ThreadState __cdecl thread_join(ThreadHandle, int*);
    void __cdecl thread_sleep(const XTime*);
    void __cdecl thread_yield();
    unsigned int __cdecl thread_hardware_concurrency();
    ThreadId __cdecl thread_id();

    ThreadState __cdecl mutex_init(MutexHandle*, MutexType);
    void __cdecl mutex_destroy(MutexHandle);
    void __cdecl mutex_init_in_situ(MutexHandle, MutexType);
    void __cdecl mutex_destroy_in_situ(MutexHandle);
    int __cdecl mutex_current_owns(MutexHandle);
    ThreadState __cdecl mutex_lock(MutexHandle);
    ThreadState __cdecl mutex_trylock(MutexHandle);
    ThreadState __cdecl mutex_timedlock(MutexHandle, const XTime*);
    ThreadState __cdecl mutex_unlock(MutexHandle);

    void* __cdecl mutex_getconcrtcs(MutexHandle);
    void __cdecl mutex_clear_owner(MutexHandle);
    void __cdecl mutex_reset_owner(MutexHandle);

    void __cdecl sharedmutex_lock_exclusive(SharedMutex*);
    void __cdecl sharedmutex_lock_shared(SharedMutex*);
    int __cdecl sharedmutex_try_lock_exclusive(SharedMutex*);
    int __cdecl sharedmutex_try_lock_shared(SharedMutex*);
    void __cdecl sharedmutex_unlock_exclusive(SharedMutex*);
    void __cdecl sharedmutex_unlock_shared(SharedMutex*);

    ThreadState __cdecl cond_init(CondHandle*);
    void __cdecl cond_destroy(CondHandle);
    void __cdecl cond_init_in_situ(CondHandle);
    void __cdecl cond_destroy_in_situ(CondHandle);
    ThreadState __cdecl cond_wait(CondHandle, MutexHandle);
    ThreadState __cdecl cond_timedwait(CondHandle, MutexHandle, const XTime*);
    ThreadState __cdecl cond_broadcast(CondHandle);
    ThreadState __cdecl cond_signal(CondHandle);
    void __cdecl cond_do_broadcast_at_thread_exit();

    typedef void(__cdecl* beginthread_proc_type)(void*);
    typedef unsigned(__stdcall* beginthreadex_proc_type)(void*);

    uintptr_t __cdecl beginthread(beginthread_proc_type _StartAddress, unsigned _StackSize, void* _ArgList);

    void __cdecl endthread(void);
    uintptr_t __cdecl beginthreadex(void* _Security, unsigned _StackSize, beginthreadex_proc_type _StartAddress,
                                    void* _ArgList, unsigned _InitFlag, unsigned* _ThrdAddr);

    void __cdecl endthreadex(unsigned _ReturnCode);
} // namespace ARLib
#endif
#endif