#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "XNative/atomic/xnative_atomic_windows.hpp"

// while writing this file a lot of work was taken from MSVC-STL's implementation of atomics
// as such, a lot of the implementation code was taken from that, since atomic operations are very tricky
// and I wanted to make sure that I got the implementation right
// for this reason, I'll include these files have got code under the STL's copyright which I'll include here:
// Copyright (c) Microsoft Corporation.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// the full license text can be found at https://github.com/microsoft/STL/blob/main/LICENSE.txt
#ifdef WINDOWS
#include "Threading.hpp"
#include <intrin.h>
#include <Windows.h>
namespace ARLib {
void atomic_lock_acquire(long& lock) {
    int curr_backoff      = 1;
    const int max_backoff = 64;
    while (_InterlockedExchange(&lock, 1) != 0) {
        while (__iso_volatile_load32(&reinterpret_cast<int&>(lock)) != 0) {
            for (int count_down = curr_backoff; count_down != 0; --count_down) { _mm_pause(); }
            curr_backoff = curr_backoff < max_backoff ? curr_backoff << 1 : max_backoff;
        }
    }
}
void atomic_lock_acquire(SharedMutex* lock) {
    sharedmutex_lock_exclusive(lock);
}
void atomic_lock_release(long& lock) {
    _InterlockedExchange(&lock, 0);
}
void atomic_lock_release(SharedMutex* lock) {
    sharedmutex_unlock_exclusive(lock);
}
struct WaitContext {
    const void* storage;
    WaitContext* next;
    WaitContext* prev;
    internal::ConditionVariable condition;
};
struct alignas(64) WaitTableEntry {
    SharedMutex lock           = { 0 };
    WaitContext wait_list_head = { nullptr, nullptr, nullptr, { 0 } };

    constexpr WaitTableEntry() noexcept = default;
};
[[nodiscard]] WaitTableEntry& atomic_wait_table_entry(const void* const storage) noexcept {
    static WaitTableEntry wait_table[256];
    auto index = reinterpret_cast<uintptr_t>(storage);
    index ^= index >> (8 * 2);
    index ^= index >> 8;
    return wait_table[index & 0xFF];
}
struct GuardedWaitContext : WaitContext {
    GuardedWaitContext(const void* storage, WaitContext* const head) noexcept :
        WaitContext{ storage, head, head->prev, { 0 } } {
        prev->next = this;
        next->prev = this;
    }
    ~GuardedWaitContext() {
        const auto next_loc = next;
        const auto prev_loc = prev;
        next->prev          = prev_loc;
        prev->next          = next_loc;
    }
    GuardedWaitContext(const GuardedWaitContext&)            = delete;
    GuardedWaitContext& operator=(const GuardedWaitContext&) = delete;
};
class SRWLockGuard {
    public:
    explicit SRWLockGuard(SharedMutex& locked) noexcept : m_locked(&locked) { sharedmutex_lock_exclusive(m_locked); }
    ~SRWLockGuard() { sharedmutex_unlock_exclusive(m_locked); }
    SRWLockGuard(const SRWLockGuard&)            = delete;
    SRWLockGuard& operator=(const SRWLockGuard&) = delete;

    private:
    SharedMutex* m_locked;
};
bool __stdcall atomic_wait_compare_16(const void* storage, void* comp, size_t, void*) noexcept {
    const auto dest              = static_cast<long long*>(const_cast<void*>(storage));
    const auto cmp               = static_cast<const long long*>(comp);
    alignas(16) long long tmp[2] = { cmp[0], cmp[1] };
    return atomic_compare_exchange_nolock(dest, tmp[1], tmp[0], tmp) != 0;
}
int atomic_wait_for(
const void* storage, void* cmp, size_t size, void* param, equal_callback_t callback, unsigned long timeout
) noexcept {
    auto& entry = atomic_wait_table_entry(storage);

    SRWLockGuard guard{ entry.lock };

    if (entry.wait_list_head.next == nullptr) {
        entry.wait_list_head.next = &entry.wait_list_head;
        entry.wait_list_head.prev = &entry.wait_list_head;
    }

    GuardedWaitContext context{ storage, &entry.wait_list_head };
    while (true) {
        if (!callback(storage, cmp, size, param)) { return TRUE; }

        if (!SleepConditionVariableSRW(
            cast<PCONDITION_VARIABLE>(&context.condition), cast<PSRWLOCK>(&entry.lock), timeout, 0
            )) {
            return FALSE;
        }

        if (timeout != 0xFFFF'FFFF) { return TRUE; }
    }
}
void __stdcall atomic_notify_one(const void* const storage) noexcept {
    auto& entry = atomic_wait_table_entry(storage);
    SRWLockGuard guard(entry.lock);
    WaitContext* context = entry.wait_list_head.next;

    if (context == nullptr) { return; }

    for (; context != &entry.wait_list_head; context = context->next) {
        if (context->storage == storage) {
            WakeAllConditionVariable(cast<PCONDITION_VARIABLE>(&context->condition));
            break;
        }
    }
}
void __stdcall atomic_notify_all(const void* const storage) noexcept {
    auto& entry = atomic_wait_table_entry(storage);
    SRWLockGuard guard{ entry.lock };
    WaitContext* context = entry.wait_list_head.next;

    if (context == nullptr) { return; }

    for (; context != &entry.wait_list_head; context = context->next) {
        if (context->storage == storage) { WakeAllConditionVariable(cast<PCONDITION_VARIABLE>(&context->condition)); }
    }
}
int __stdcall atomic_wait_nolock(
volatile void* const storage, void* const comparand, const size_t size, const unsigned long timeout
) {
    const auto result = WaitOnAddress(storage, comparand, size, timeout);
    return result;
}
void atomic_notify_all_nolock(const void* const storage) noexcept {
    WakeByAddressAll(const_cast<void*>(storage));
}
void atomic_notify_one_nolock(const void* const storage) noexcept {
    WakeByAddressSingle(const_cast<void*>(storage));
}
void atomic_store_nolock(volatile char* addend, char value) {
    _InterlockedExchange8(addend, value);
}
void atomic_store_nolock(volatile short* addend, short value) {
    _InterlockedExchange16(addend, value);
}
void atomic_store_nolock(volatile int* addend, int value) {
    _InterlockedExchange(reinterpret_cast<volatile long*>(addend), static_cast<long>(value));
}
void atomic_store_nolock(volatile long long* addend, long long value) {
    _InterlockedExchange64(addend, value);
}
char atomic_load_nolock(const volatile char* addr) {
    return __iso_volatile_load8(addr);
}
short atomic_load_nolock(const volatile short* addr) {
    return __iso_volatile_load16(addr);
}
int atomic_load_nolock(const volatile int* addr) {
    return __iso_volatile_load32(addr);
}
long long atomic_load_nolock(const volatile long long* addr) {
    return __iso_volatile_load64(addr);
}
char atomic_exchange_nolock(volatile char* addr, char value) {
    return _InterlockedExchange8(addr, value);
}
short atomic_exchange_nolock(volatile short* addr, short value) {
    return _InterlockedExchange16(addr, value);
}
int atomic_exchange_nolock(volatile int* addr, int value) {
    return _InterlockedExchange(reinterpret_cast<volatile long*>(addr), static_cast<long>(value));
}
long long atomic_exchange_nolock(volatile long long* addr, long long value) {
    return _InterlockedExchange64(addr, value);
}
char atomic_compare_exchange_nolock(volatile char* addr, char value, char comparand) {
    return _InterlockedCompareExchange8(addr, value, comparand);
}
short atomic_compare_exchange_nolock(volatile short* addr, short value, short comparand) {
    return _InterlockedCompareExchange16(addr, value, comparand);
}
int atomic_compare_exchange_nolock(volatile int* addr, int value, int comparand) {
    return _InterlockedCompareExchange(
    reinterpret_cast<volatile long*>(addr), static_cast<long>(value), static_cast<long>(comparand)
    );
}
long long atomic_compare_exchange_nolock(volatile long long* addr, long long value, long long comparand) {
    return _InterlockedCompareExchange64(addr, value, comparand);
}
unsigned char atomic_compare_exchange_nolock(
volatile long long* addr, long long value_high, long long value_low, long long* comparand
) {
    return _InterlockedCompareExchange128(addr, value_high, value_low, comparand);
}
void memory_barrier() {
    _ReadWriteBarrier();
}
void pause_sync() {
    _mm_pause();
}
char atomic_integral_op_nolock(volatile char* addr, char value, AtomicIntegralOp op_type) {
    switch (op_type) {
        case AtomicIntegralOp::ExchAdd:
            return _InterlockedExchangeAdd8(addr, value);
        case AtomicIntegralOp::And:
            return _InterlockedAnd8(addr, value);
        case AtomicIntegralOp::Or:
            return _InterlockedOr8(addr, value);
        case AtomicIntegralOp::Xor:
            return _InterlockedXor8(addr, value);
        case AtomicIntegralOp::Inc:
            return _InterlockedExchangeAdd8(addr, 1);
        case AtomicIntegralOp::Dec:
            return _InterlockedExchangeAdd8(addr, -1);
    }
    unreachable;
}
short atomic_integral_op_nolock(volatile short* addr, short value, AtomicIntegralOp op_type) {
    switch (op_type) {
        case AtomicIntegralOp::ExchAdd:
            return _InterlockedExchangeAdd16(addr, value);
        case AtomicIntegralOp::And:
            return _InterlockedAnd16(addr, value);
        case AtomicIntegralOp::Or:
            return _InterlockedOr16(addr, value);
        case AtomicIntegralOp::Xor:
            return _InterlockedXor16(addr, value);
        case AtomicIntegralOp::Inc:
            return _InterlockedIncrement16(addr);
        case AtomicIntegralOp::Dec:
            return _InterlockedDecrement16(addr);
    }
    unreachable;
}
int atomic_integral_op_nolock(volatile int* addr, int value, AtomicIntegralOp op_type) {
    switch (op_type) {
        case AtomicIntegralOp::ExchAdd:
            return _InterlockedExchangeAdd(reinterpret_cast<volatile long*>(addr), static_cast<long>(value));
        case AtomicIntegralOp::And:
            return _InterlockedAnd(reinterpret_cast<volatile long*>(addr), static_cast<long>(value));
        case AtomicIntegralOp::Or:
            return _InterlockedOr(reinterpret_cast<volatile long*>(addr), static_cast<long>(value));
        case AtomicIntegralOp::Xor:
            return _InterlockedXor(reinterpret_cast<volatile long*>(addr), static_cast<long>(value));
        case AtomicIntegralOp::Inc:
            return _InterlockedIncrement(reinterpret_cast<volatile long*>(addr));
        case AtomicIntegralOp::Dec:
            return _InterlockedDecrement(reinterpret_cast<volatile long*>(addr));
    }
    unreachable;
}
long long atomic_integral_op_nolock(volatile long long* addr, long long value, AtomicIntegralOp op_type) {
    switch (op_type) {
        case AtomicIntegralOp::ExchAdd:
            return _InterlockedExchangeAdd64(addr, value);
        case AtomicIntegralOp::And:
            return _InterlockedAnd64(addr, value);
        case AtomicIntegralOp::Or:
            return _InterlockedOr64(addr, value);
        case AtomicIntegralOp::Xor:
            return _InterlockedXor64(addr, value);
        case AtomicIntegralOp::Inc:
            return _InterlockedIncrement64(addr);
        case AtomicIntegralOp::Dec:
            return _InterlockedDecrement64(addr);
    }
    unreachable;
}
}    // namespace ARLib
#endif
