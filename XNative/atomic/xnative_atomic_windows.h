#pragma once
#include "../../Compat.h"
#include "../../ThreadBase.h"
#include "../../Concepts.h"
#ifdef WINDOWS
    #if not defined(ATOMIC_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
        #error "Don't include the XNative files directly. Use Atomic.h"
    #endif
namespace ARLib {
class Mutex;
template <class T>
struct AtomicPadded {
    alignas(sizeof(T)) mutable T value;
};
template <typename T>
struct AtomicTypeProvider {
    using Storage = AtomicPadded<T>;
    using Lock    = long;
};
template <typename T>
struct AtomicTypeProvider<T&> {
    using Storage = T&;
    using Lock    = SharedMutex;
};
void atomic_lock_acquire(long& lock);
void atomic_lock_acquire(SharedMutex* lock);
void atomic_lock_release(long& lock);
void atomic_lock_release(SharedMutex* lock);
template <typename Lock>
bool __stdcall atomic_wait_compare(const void* storage, void* cmp, size_t size, void* raw) {
    Lock& lock = *static_cast<Lock*>(raw);
    atomic_lock_acquire(lock);
    const auto res = memcmp(storage, cmp, size);
    atomic_lock_release(lock);
    return res == 0;
}
bool __stdcall atomic_wait_compare_16(const void* storage, void* cmp, size_t, void*) noexcept;
using equal_callback_t = bool(__stdcall*)(const void*, void*, size_t, void*) noexcept;
int atomic_wait_for(
const void* storage, void* cmp, size_t size, void* param, equal_callback_t callback, unsigned long timeout
) noexcept;
void __stdcall atomic_notify_all(const void* const storage) noexcept;
void __stdcall atomic_notify_one(const void* const storage) noexcept;
int __stdcall atomic_wait_nolock(
volatile void* const storage, void* const comparand, const size_t size, const unsigned long timeout
);
void __stdcall atomic_notify_all_nolock(const void* const storage) noexcept;
void __stdcall atomic_notify_one_nolock(const void* const storage) noexcept;
template <Integral Int, class T>
Int reinterpret_cast_atomic(const T& source) noexcept {
    if constexpr (Integral<T> && sizeof(Int) == sizeof(T)) {
        return static_cast<Int>(source);
    } else if constexpr (IsPointerV<T> && sizeof(Int) == sizeof(T)) {
        return reinterpret_cast<Int>(source);
    } else {
        Int res{};
        ARLib::memcpy(&res, addressof(source), sizeof(source));
        return res;
    }
}
template <Integral Int, class T>
volatile Int* addressof_atomic(T& source) noexcept {
    return &reinterpret_cast<volatile Int&>(source);
}
template <Integral Int, class T>
const volatile Int* addressof_atomic(const T& source) noexcept {
    return &reinterpret_cast<const volatile Int&>(source);
}
template <Integral Int, class T1, class T2>
auto storage_and_bytes(T1& storage, T2 value) {
    return Pair{ addressof_atomic<Int>(storage), reinterpret_cast_atomic<Int>(value) };
}
void atomic_store_nolock(volatile char* addend, char value);
void atomic_store_nolock(volatile short* addend, short value);
void atomic_store_nolock(volatile int* addend, int value);
void atomic_store_nolock(volatile long long* addend, long long value);

char atomic_load_nolock(const volatile char* addr);
short atomic_load_nolock(const volatile short* addr);
int atomic_load_nolock(const volatile int* addr);
long long atomic_load_nolock(const volatile long long* addr);

char atomic_exchange_nolock(volatile char* addr, char value);
short atomic_exchange_nolock(volatile short* addr, short value);
int atomic_exchange_nolock(volatile int* addr, int value);
long long atomic_exchange_nolock(volatile long long* addr, long long value);

char atomic_compare_exchange_nolock(volatile char* addr, char value, char comparand);
short atomic_compare_exchange_nolock(volatile short* addr, short value, short comparand);
int atomic_compare_exchange_nolock(volatile int* addr, int value, int comparand);
long long atomic_compare_exchange_nolock(volatile long long* addr, long long value, long long comparand);
unsigned char atomic_compare_exchange_nolock(
volatile long long* addr, long long value_high, long long value_low, long long* comparand
);

enum class AtomicIntegralOp { ExchAdd, And, Or, Xor, Inc, Dec };
char atomic_integral_op_nolock(volatile char* addr, char value, AtomicIntegralOp op_type);
short atomic_integral_op_nolock(volatile short* addr, short value, AtomicIntegralOp op_type);
int atomic_integral_op_nolock(volatile int* addr, int value, AtomicIntegralOp op_type);
long long atomic_integral_op_nolock(volatile long long* addr, long long value, AtomicIntegralOp op_type);

void memory_barrier();
void pause_sync();
template <typename Lock>
struct AtomicLockGuard {
    Lock m_lock;
    public:
    explicit AtomicLockGuard(Lock& lock) noexcept : m_lock(lock) { atomic_lock_acquire(m_lock); }
    ~AtomicLockGuard() { atomic_lock_release(m_lock); }
    AtomicLockGuard(const AtomicLockGuard&)            = delete;
    AtomicLockGuard& operator=(const AtomicLockGuard&) = delete;
};
template <typename T>
struct AtomicStorage {
    using Val       = RemoveReferenceT<T>;
    using GuardType = AtomicLockGuard<typename AtomicTypeProvider<T>::Lock>;
    T m_storage{};
    mutable typename AtomicTypeProvider<T>::Lock m_lock{};

    public:
    AtomicStorage() = default;
    constexpr AtomicStorage(ConditionalT<IsReference<T>::value, T, const Val> value) noexcept :
        m_storage(value), m_lock{} {}
    void store(const Val value) noexcept {
        GuardType lock{ m_lock };
        m_storage = value;
    }
    Val load() const noexcept {
        GuardType lock{ m_lock };
        Val local{ m_storage };
        return local;
    }
    Val exchange(const Val value) noexcept {
        GuardType lock{ m_lock };
        Val result{ m_storage };
        m_storage = value;
        return result;
    }
    bool is_lock_free() const noexcept { return false; }
    bool compare_exchange_strong(Val& expected, const Val desired) noexcept {
        const auto storage_ptr  = addressof(m_storage);
        const auto expected_ptr = addressof(expected);
        bool result;
        GuardType lock{ m_lock };
        result = memcmp(storage_ptr, expected_ptr, sizeof(Val)) == 0;
        if (result) {
            memcpy(storage_ptr, addressof(desired), sizeof(Val));
        } else {
            memcpy(expected_ptr, storage_ptr, sizeof(Val));
        }
        return result;
    }
    void wait(Val expected) const noexcept {
        const auto storage_ptr  = addressof(m_storage);
        const auto expected_ptr = addressof(expected);
        while (true) {
            {
                GuardType lock{ m_lock };
                if (memcmp(storage_ptr, expected_ptr, sizeof(Val)) == 0) { return; }
            }
            atomic_wait_for(
            storage_ptr, expected_ptr, sizeof(Val), &m_lock, atomic_wait_compare<decltype(m_lock)>, 0xFFFF'FFFF
            );
        }
    }
    void notify_one() noexcept { atomic_notify_one(addressof(m_storage)); }
    void notify_all() noexcept { atomic_notify_all(addressof(m_storage)); }
};
template <size_t N, typename T>
concept AtomicStorageLockFreeSize = (N == 8 || N == 4 || N == 2 || N == 1);
template <typename T>
requires AtomicStorageLockFreeSize<sizeof(T), T>
struct AtomicStorage<T> {
    constexpr static inline size_t TYPE_SIZE  = sizeof(T);
    constexpr static inline size_t TYPE_INDEX = []() {
        switch (TYPE_SIZE) {
            case 8:
                return 3;
            case 4:
                return 2;
            case 2:
                return 1;
            case 1:
                return 0;
        }
    }();
    using Conv = typename TypeArray<char, short, int, long long>::template At<TYPE_INDEX>;
    using Val  = RemoveReferenceT<T>;
    T m_storage;

    public:
    AtomicStorage() = default;
    constexpr AtomicStorage(ConditionalT<IsReference<T>::value, T, const Val> value) noexcept : m_storage(value) {}
    void store(const Val value) noexcept {
        const auto [mem, bytes] = storage_and_bytes<Conv>(m_storage, value);
        atomic_store_nolock(mem, bytes);
    }
    Val load() const noexcept {
        const auto mem = addressof_atomic<Conv>(m_storage);
        Conv bytes     = atomic_load_nolock(mem);
        memory_barrier();
        return reinterpret_cast<Val&>(bytes);
    }
    Val exchange(const Val value) noexcept {
        const auto [mem, bytes] = storage_and_bytes<Conv>(m_storage, value);
        Conv result             = atomic_exchange_nolock(mem, bytes);
        return reinterpret_cast<Val&>(result);
    }
    bool is_lock_free() const noexcept { return true; }
    bool compare_exchange_strong(Val& expected, const Val desired) noexcept {
        const auto [mem, bytes] = storage_and_bytes<Conv>(m_storage, desired);
        Conv expected_bytes     = reinterpret_cast_atomic<Conv>(expected);
        Conv prev_bytes         = atomic_compare_exchange_nolock(mem, bytes, expected_bytes);
        if (prev_bytes == expected_bytes) return true;
        reinterpret_cast<Conv&>(expected) = prev_bytes;
        return false;
    }
    void wait(Val expected) const noexcept {
        const auto storage        = addressof(m_storage);
        auto expected_bytes = reinterpret_cast_atomic<Conv>(expected);
        while (true) {
            const Conv observed_bytes = reinterpret_cast_atomic<Conv>(load());
            if (expected_bytes != observed_bytes) { return; }
            atomic_wait_nolock(reinterpret_cast<volatile void*>(const_cast<T*>(storage)), &expected_bytes, sizeof(Conv), 0xFFFFFFFF);
        }
    }
    void notify_one() noexcept { atomic_notify_one_nolock(addressof(m_storage)); }
    void notify_all() noexcept { atomic_notify_all_nolock(addressof(m_storage)); }
};
template <typename T>
requires(sizeof(T) == 16)
struct AtomicStorage<T> {
    using Val = RemoveReferenceT<T>;
    T m_storage;
    struct Int128 {
        alignas(16) long long low;
        long long high;
    };

    public:
    AtomicStorage() = default;
    constexpr AtomicStorage(ConditionalT<IsReference<T&>::value, T&, const Val> value) noexcept : m_storage(value) {}
    void store(const Val value) noexcept { exchange(value); }
    Val load() const noexcept {
        auto storage_ptr = const_cast<long long*>(addressof_atomic<const long long>(m_storage));
        Int128 result{};
        atomic_compare_exchange_nolock(storage_ptr, 0, 0, &result.low);
        return reinterpret_cast<Val&>(result);
    }
    Val exchange(const Val value) noexcept {
        Val result{ value };
        while (!compare_exchange_strong(result, value)) {}
        return result;
    }
    bool is_lock_free() const noexcept { return true; }
    bool compare_exchange_strong(Val& expected, const Val desired) noexcept {
        Int128 desired_bytes{};
        ARLib::memcpy(&desired_bytes, addressof(desired), sizeof(Val));
        Int128 expected_temp{};
        ARLib::memcpy(&expected_temp, addressof(expected), sizeof(Val));
        unsigned char result = atomic_compare_exchange_nolock(
        &reinterpret_cast<long long&>(m_storage), desired_bytes.high, desired_bytes.low, &expected_temp.low
        );
        if (result == 0) { ARLib::memcpy(addressof(expected), &expected_temp, sizeof(Val)); }
        return result != 0;
    }
    void wait(Val expected) const noexcept {
        const auto storage_ptr  = addressof(m_storage);
        const auto expected_ptr = addressof(expected);
        Int128 expected_bytes   = reinterpret_cast<const Int128&>(expected);
        for (;;) {
            const Val observed    = load();
            Int128 observed_bytes = reinterpret_cast<const Int128&>(observed);
            if (observed_bytes.low != expected_bytes.low || observed_bytes.high != expected_bytes.high) { return; }
            atomic_wait_for(storage_ptr, expected_ptr, sizeof(Val), nullptr, &atomic_wait_compare_16, 0xFFFF'FFFF);
        }
    }
    void notify_one() noexcept { atomic_notify_one(addressof(m_storage)); }
    void notify_all() noexcept { atomic_notify_all(addressof(m_storage)); }
};
template <Integral T>
struct AtomicIntegralStorage : public AtomicStorage<T> {
    using Base = AtomicStorage<T>;
    using Val  = typename AtomicStorage<T>::Val;
    using Conv = typename AtomicStorage<T>::Conv;
    Val fetch_add(const Val operand) noexcept {
        Conv result =
        atomic_integral_op_nolock(addressof_atomic<Conv>(this->m_storage), operand, AtomicIntegralOp::ExchAdd);
        return static_cast<Val>(result);
    }
    Val fetch_and(const Val operand) noexcept {
        Conv result =
        atomic_integral_op_nolock(addressof_atomic<Conv>(this->m_storage), operand, AtomicIntegralOp::And);
        return static_cast<Val>(result);
    }
    Val fetch_or(const Val operand) noexcept {
        Conv result = atomic_integral_op_nolock(addressof_atomic<Conv>(this->m_storage), operand, AtomicIntegralOp::Or);
        return static_cast<Val>(result);
    }
    Val fetch_xor(const Val operand) noexcept {
        Conv result =
        atomic_integral_op_nolock(addressof_atomic<Conv>(this->m_storage), operand, AtomicIntegralOp::Xor);
        return static_cast<Val>(result);
    }
    Val operator++(int) noexcept {
        Conv after = atomic_integral_op_nolock(addressof_atomic<Conv>(this->m_storage), 1, AtomicIntegralOp::Inc);
        ++after;
        return static_cast<Val>(after);
    }
    Val operator++() noexcept {
        return static_cast<Val>(
        atomic_integral_op_nolock(addressof_atomic<Conv>(this->m_storage), 1, AtomicIntegralOp::Inc)
        );
    }
    Val operator--(int) noexcept {
        Conv after = atomic_integral_op_nolock(addressof_atomic<Conv>(this->m_storage), -1, AtomicIntegralOp::Dec);
        --after;
        return static_cast<Val>(after);
    }
    Val operator--() noexcept {
        return static_cast<Val>(
        atomic_integral_op_nolock(addressof_atomic<Conv>(this->m_storage), -1, AtomicIntegralOp::Dec)
        );
    }
};
}    // namespace ARLib
#endif
