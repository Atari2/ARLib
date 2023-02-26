#pragma once
#define ATOMIC_INCLUDED__
#include "XNative/atomic/xnative_atomic_merge.h"
#include "Concepts.h"
namespace ARLib {
// Atomic class made around memory_order_seq_cst
template <typename T>
concept AtomicRequirements =
(IsTriviallyCopiableV<T> && CopyConstructibleV<T> && MoveConstructibleV<T> && CopyAssignableV<T> && MoveAssignableV<T>);
template <AtomicRequirements T>
class AtomicBase {
    protected:
    AtomicStorage<T> m_storage;
    public:
    AtomicBase() noexcept(NothrowDefaultConstructibleV<T>) = default;
    AtomicBase(T value) noexcept : m_storage{ value } {}
    AtomicBase(const AtomicBase&) = delete;
    T operator=(T val) noexcept {
        m_storage.store(val);
        return val;
    }
    AtomicBase& operator=(const AtomicBase&) = delete;
    bool is_lock_free() const noexcept { return m_storage.is_lock_free(); }
    void store(T val) noexcept { m_storage.store(val); }
    T load() const noexcept { return m_storage.load(); }
    operator T() const noexcept { return m_storage.load(); }
    T exchange(T val) noexcept { return m_storage.exchange(val); }
    bool compare_exchange_weak(T& expected, T desired) noexcept {
        return m_storage.compare_exchange_strong(expected, desired);
    }
    bool compare_exchange_strong(T& expected, T desired) noexcept {
        return m_storage.compare_exchange_strong(expected, desired);
    }
    void wait(T old) const noexcept { m_storage.wait(old); }
    void notify_all() noexcept { m_storage.notify_all(); }
    void notify_one() noexcept { m_storage.notify_one(); }
};
template <AtomicRequirements T>
requires Integral<T>
class IntegralAtomicBase {
    protected:
    AtomicIntegralStorage<T> m_storage;
    static T negate(const T val) noexcept { return static_cast<T>(0U - static_cast<MakeUnsignedT<T>>(val)); }
    public:
    IntegralAtomicBase() noexcept = default;
    IntegralAtomicBase(T value) noexcept : m_storage{ value } {}
    IntegralAtomicBase(const IntegralAtomicBase&) = delete;
    T operator=(T val) noexcept {
        this->m_storage.store(val);
        return val;
    }
    IntegralAtomicBase& operator=(const IntegralAtomicBase&) = delete;
    bool is_lock_free() const noexcept { return this->m_storage.is_lock_free(); }
    void store(T val) noexcept { this->m_storage.store(val); }
    T load() const noexcept { return this->m_storage.load(); }
    operator T() const noexcept { return this->m_storage.load(); }
    T exchange(T val) noexcept { return this->m_storage.exchange(val); }
    bool compare_exchange_weak(T& expected, T desired) noexcept {
        return this->m_storage.compare_exchange_strong(expected, desired);
    }
    bool compare_exchange_strong(T& expected, T desired) noexcept {
        return this->m_storage.compare_exchange_strong(expected, desired);
    }
    void wait(T old) const noexcept { this->m_storage.wait(old); }
    void notify_all() noexcept { this->m_storage.notify_all(); }
    void notify_one() noexcept { this->m_storage.notify_one(); }
    T fetch_add(T arg) noexcept { return m_storage.fetch_add(arg); }
    T fetch_sub(T arg) noexcept { return m_storage.fetch_add(negate(arg)); }
    T fetch_and(T arg) noexcept { return m_storage.fetch_and(arg); }
    T fetch_or(T arg) noexcept { return m_storage.fetch_or(arg); }
    T fetch_xor(T arg) noexcept { return m_storage.fetch_xor(arg); }
    T operator++() noexcept { return m_storage++; }
    T operator++(int) noexcept { return ++m_storage; }
    T operator--() noexcept { return m_storage--; }
    T operator--(int) noexcept { return --m_storage; }
    T operator+=(T arg) noexcept { return static_cast<T>(m_storage.fetch_add(arg) + arg); }
    T operator-=(T arg) noexcept { return static_cast<T>(m_storage.fetch_add(negate(arg)) - arg); }
    T operator&=(T arg) noexcept { return static_cast<T>(m_storage.fetch_and(arg) & arg); }
    T operator|=(T arg) noexcept { return static_cast<T>(m_storage.fetch_or(arg) | arg); }
    T operator^=(T arg) noexcept { return static_cast<T>(m_storage.fetch_xor(arg) ^ arg); }
};
template <typename T>
class PointerAtomicBase {
    using Ptr = T*;
    protected:
    AtomicIntegralStorage<uintptr_t> m_storage;
    static uintptr_t negate(const ptrdiff_t val) noexcept {
        return static_cast<uintptr_t>(0U - static_cast<MakeUnsignedT<uintptr_t>>(val));
    }
    static uintptr_t to_uintptr(Ptr val) { return reinterpret_cast<uintptr_t>(val); }
    static Ptr from_uintptr(uintptr_t val) { return reinterpret_cast<Ptr>(val); }
    public:
    PointerAtomicBase() noexcept = default;
    PointerAtomicBase(Ptr value) noexcept : m_storage{ to_uintptr(value) } {}
    PointerAtomicBase(const PointerAtomicBase&) = delete;
    Ptr operator=(Ptr val) noexcept {
        this->m_storage.store(to_uintptr(val));
        return val;
    }
    PointerAtomicBase& operator=(const PointerAtomicBase&) = delete;
    bool is_lock_free() const noexcept { return this->m_storage.is_lock_free(); }
    void store(Ptr val) noexcept { this->m_storage.store(to_uintptr(val)); }
    Ptr load() const noexcept { return from_uintptr(this->m_storage.load()); }
    operator Ptr() const noexcept { return from_uintptr(this->m_storage.load()); }
    Ptr exchange(Ptr val) noexcept { return from_uintptr(this->m_storage.exchange(to_uintptr(val))); }
    bool compare_exchange_weak(T& expected, T desired) noexcept {
        return this->m_storage.compare_exchange_strong(expected, desired);
    }
    bool compare_exchange_strong(T& expected, T desired) noexcept {
        return this->m_storage.compare_exchange_strong(expected, desired);
    }
    void wait(Ptr old) const noexcept { this->m_storage.wait(to_uintptr(old)); }
    void notify_all() noexcept { this->m_storage.notify_all(); }
    void notify_one() noexcept { this->m_storage.notify_one(); }
    Ptr fetch_add(ptrdiff_t arg) noexcept { return from_uintptr(m_storage.fetch_add(arg)); }
    Ptr fetch_sub(ptrdiff_t arg) noexcept { return from_uintptr(m_storage.fetch_add(negate(arg))); }
    Ptr fetch_and(ptrdiff_t arg) noexcept { return from_uintptr(m_storage.fetch_and(arg)); }
    Ptr fetch_or(ptrdiff_t arg) noexcept { return from_uintptr(m_storage.fetch_or(arg)); }
    Ptr fetch_xor(ptrdiff_t arg) noexcept { return from_uintptr(m_storage.fetch_xor(arg)); }
    Ptr operator++() noexcept { return from_uintptr(m_storage++); }
    Ptr operator++(int) noexcept { return from_uintptr(++m_storage); }
    Ptr operator--() noexcept { return from_uintptr(m_storage--); }
    Ptr operator--(int) noexcept { return from_uintptr(--m_storage); }
    Ptr operator+=(ptrdiff_t arg) noexcept { return from_uintptr(m_storage.fetch_add(arg) + arg); }
    Ptr operator-=(ptrdiff_t arg) noexcept { return from_uintptr(m_storage.fetch_add(negate(arg)) - arg); }
};
template <AtomicRequirements T>
class Atomic : public AtomicBase<T> {
    using Base = AtomicBase<T>;
    public:
    Atomic() noexcept(NothrowDefaultConstructibleV<T>) = default;
    Atomic(T value) noexcept : Base{ value } {}
    Atomic(const Atomic&) = delete;
    T operator=(T val) noexcept {
        Base::m_storage.store(val);
        return val;
    }
    Atomic& operator=(const Atomic&) = delete;
};
template <AtomicRequirements T>
requires Integral<T>
class Atomic<T> : public IntegralAtomicBase<T> {
    using Base = IntegralAtomicBase<T>;
    public:
    Atomic() noexcept(NothrowDefaultConstructibleV<T>) = default;
    Atomic(T value) noexcept : Base{ value } {}
    Atomic(const Atomic&) = delete;
    T operator=(T val) noexcept {
        this->m_storage.store(val);
        return val;
    }
    Atomic& operator=(const Atomic&) = delete;
};
template <typename P>
class Atomic<P*> : public PointerAtomicBase<P> {
    using Base = PointerAtomicBase<P>;
    public:
    Atomic() noexcept = default;
    Atomic(P* value) noexcept : Base{ value } {}
    Atomic(const Atomic&) = delete;
    P* operator=(P* val) noexcept {
        this->m_storage.store(val);
        return val;
    }
    Atomic& operator=(const Atomic&) = delete;
};
}    // namespace ARLib