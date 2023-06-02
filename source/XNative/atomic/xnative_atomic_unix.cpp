#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "XNative/atomic/xnative_atomic_unix.hpp"
#ifdef UNIX_OR_MINGW
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <immintrin.h>
#include "XNative/chrono/xnative_chrono_merge.hpp"
namespace ARLib {
void atomic_lock_acquire(long& lock) {
    int curr_backoff      = 1;
    const int max_backoff = 64;
    while (__atomic_test_and_set(&lock, __ATOMIC_SEQ_CST) != 0) {
        int ret = 1;
        __atomic_load(&reinterpret_cast<int&>(lock), &ret, __ATOMIC_SEQ_CST);
        while (ret != 0) {
            __atomic_load(&reinterpret_cast<int&>(lock), &ret, __ATOMIC_SEQ_CST);
            for (int count_down = curr_backoff; count_down != 0; --count_down) { _mm_pause(); }
            curr_backoff = curr_backoff < max_backoff ? curr_backoff << 1 : max_backoff;
        }
    }
}
void atomic_lock_acquire(PthreadRWLock* lock) {
    pthread_rwlock_wrlock(lock);
}
void atomic_lock_release(long& lock) {
    __atomic_clear(&lock, __ATOMIC_SEQ_CST);
}
void atomic_lock_release(PthreadRWLock* lock) {
    pthread_rwlock_unlock(lock);
}
struct WaitContext {
    const void* storage;
    WaitContext* next;
    WaitContext* prev;
    PthreadCond condition;
};
struct alignas(64) WaitTableEntry {
    PthreadMutex lock          = ARLIB_PTHREAD_MUTEX_INITIALIZER;
    WaitContext wait_list_head = { nullptr, nullptr, nullptr, ARLIB_PTHREAD_COND_INITIALIZER };

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
    GuardedWaitContext(const void* s_storage, WaitContext* const head) noexcept :
        WaitContext{ s_storage, head, head->prev, ARLIB_PTHREAD_COND_INITIALIZER } {
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
    explicit SRWLockGuard(PthreadMutex& locked) noexcept : m_locked(&locked) { pthread_mutex_lock(m_locked); }
    ~SRWLockGuard() { pthread_mutex_unlock(m_locked); }
    SRWLockGuard(const SRWLockGuard&)            = delete;
    SRWLockGuard& operator=(const SRWLockGuard&) = delete;

    private:
    PthreadMutex* m_locked;
};
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
    for (;;) {
        if (!callback(storage, cmp, size, param)) {    // note: under lock to prevent lost wakes
            return true;
        }
        TimeSpec spec{ static_cast<long>(timeout), 0 };
        if (!pthread_cond_timedwait(&context.condition, &entry.lock, &spec)) { return false; }

        if (timeout != 0xFFFF'FFFF) {
            // spurious wake to recheck the clock
            return true;
        }
    }
}
void atomic_notify_one(const void* const storage) noexcept {
    auto& entry = atomic_wait_table_entry(storage);
    SRWLockGuard guard(entry.lock);
    WaitContext* context = entry.wait_list_head.next;

    if (context == nullptr) { return; }

    for (; context != &entry.wait_list_head; context = context->next) {
        if (context->storage == storage) {
            pthread_cond_signal(&context->condition);
            break;
        }
    }
}
void atomic_notify_all(const void* const storage) noexcept {
    auto& entry = atomic_wait_table_entry(storage);
    SRWLockGuard guard(entry.lock);
    WaitContext* context = entry.wait_list_head.next;

    if (context == nullptr) { return; }

    for (; context != &entry.wait_list_head; context = context->next) {
        if (context->storage == storage) { pthread_cond_signal(&context->condition); }
    }
}
int atomic_wait_nolock(volatile void* const storage, void* const comparand, const unsigned long timeout_) {
    TimeSpec timeout = { static_cast<long>(timeout_), 0 };
    auto res         = syscall(SYS_futex, storage, FUTEX_WAIT_PRIVATE, comparand, &timeout, 0, 0);
    return static_cast<int>(res);
}
void atomic_notify_all_nolock(const void* const storage) noexcept {
    syscall(SYS_futex, storage, FUTEX_WAKE_PRIVATE, NumberTraits<int>::max, 0, 0, 0);
}
void atomic_notify_one_nolock(const void* const storage) noexcept {
    syscall(SYS_futex, storage, FUTEX_WAKE_PRIVATE, 1, 0, 0, 0);
}
void atomic_store_nolock(volatile char* addend, char value) {
    __atomic_store_n(addend, value, __ATOMIC_SEQ_CST);
}
void atomic_store_nolock(volatile short* addend, short value) {
    __atomic_store_n(addend, value, __ATOMIC_SEQ_CST);
}
void atomic_store_nolock(volatile int* addend, int value) {
    __atomic_store_n(addend, value, __ATOMIC_SEQ_CST);
}
void atomic_store_nolock(volatile long long* addend, long long value) {
    __atomic_store_n(addend, value, __ATOMIC_SEQ_CST);
}
char atomic_load_nolock(const volatile char* addr) {
    char ret = 0;
    __atomic_load(addr, &ret, __ATOMIC_SEQ_CST);
    return ret;
}
short atomic_load_nolock(const volatile short* addr) {
    short ret = 0;
    __atomic_load(addr, &ret, __ATOMIC_SEQ_CST);
    return ret;
}
int atomic_load_nolock(const volatile int* addr) {
    int ret = 0;
    __atomic_load(addr, &ret, __ATOMIC_SEQ_CST);
    return ret;
}
long long atomic_load_nolock(const volatile long long* addr) {
    long long ret = 0;
    __atomic_load(addr, &ret, __ATOMIC_SEQ_CST);
    return ret;
}
char atomic_exchange_nolock(volatile char* addr, char value) {
    return __atomic_exchange_n(addr, value, __ATOMIC_SEQ_CST);
}
short atomic_exchange_nolock(volatile short* addr, short value) {
    return __atomic_exchange_n(addr, value, __ATOMIC_SEQ_CST);
}
int atomic_exchange_nolock(volatile int* addr, int value) {
    return __atomic_exchange_n(addr, value, __ATOMIC_SEQ_CST);
}
long long atomic_exchange_nolock(volatile long long* addr, long long value) {
    return __atomic_exchange_n(addr, value, __ATOMIC_SEQ_CST);
}
char atomic_compare_exchange_nolock(volatile char* addr, char value, char comparand) {
    __atomic_compare_exchange_n(addr, &value, comparand, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return value;
}
short atomic_compare_exchange_nolock(volatile short* addr, short value, short comparand) {
    __atomic_compare_exchange_n(addr, &value, comparand, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return value;
}
int atomic_compare_exchange_nolock(volatile int* addr, int value, int comparand) {
    __atomic_compare_exchange_n(addr, &value, comparand, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return value;
}
long long atomic_compare_exchange_nolock(volatile long long* addr, long long value, long long comparand) {
    __atomic_compare_exchange_n(addr, &value, comparand, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return value;
}
void memory_barrier() {
    __sync_synchronize();
}
void pause_sync() {
    _mm_pause();
}
char atomic_integral_op_nolock(volatile char* addr, char value, AtomicIntegralOp op_type) {
    switch (op_type) {
        case AtomicIntegralOp::ExchAdd:
            return __atomic_fetch_add(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::And:
            return __atomic_fetch_and(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Or:
            return __atomic_fetch_or(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Xor:
            return __atomic_fetch_xor(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Inc:
            return __atomic_fetch_add(addr, 1, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Dec:
            return __atomic_fetch_sub(addr, 1, __ATOMIC_SEQ_CST);
    }
    arlib_unreachable;
}
short atomic_integral_op_nolock(volatile short* addr, short value, AtomicIntegralOp op_type) {
    switch (op_type) {
        case AtomicIntegralOp::ExchAdd:
            return __atomic_fetch_add(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::And:
            return __atomic_fetch_and(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Or:
            return __atomic_fetch_or(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Xor:
            return __atomic_fetch_xor(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Inc:
            return __atomic_fetch_add(addr, 1, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Dec:
            return __atomic_fetch_sub(addr, 1, __ATOMIC_SEQ_CST);
    }
    arlib_unreachable;
}
int atomic_integral_op_nolock(volatile int* addr, int value, AtomicIntegralOp op_type) {
    switch (op_type) {
        case AtomicIntegralOp::ExchAdd:
            return __atomic_fetch_add(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::And:
            return __atomic_fetch_and(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Or:
            return __atomic_fetch_or(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Xor:
            return __atomic_fetch_xor(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Inc:
            return __atomic_fetch_add(addr, 1, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Dec:
            return __atomic_fetch_sub(addr, 1, __ATOMIC_SEQ_CST);
    }
    arlib_unreachable;
}
long long atomic_integral_op_nolock(volatile long long* addr, long long value, AtomicIntegralOp op_type) {
    switch (op_type) {
        case AtomicIntegralOp::ExchAdd:
            return __atomic_fetch_add(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::And:
            return __atomic_fetch_and(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Or:
            return __atomic_fetch_or(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Xor:
            return __atomic_fetch_xor(addr, value, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Inc:
            return __atomic_fetch_add(addr, 1, __ATOMIC_SEQ_CST);
        case AtomicIntegralOp::Dec:
            return __atomic_fetch_sub(addr, 1, __ATOMIC_SEQ_CST);
    }
    arlib_unreachable;
}
}    // namespace ARLib
#endif
