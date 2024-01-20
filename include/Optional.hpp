#pragma once
#include "Assertion.hpp"
#include "Concepts.hpp"
#include "HashBase.hpp"
#include "Ordering.hpp"
#include "PrintInfo.hpp"
namespace ARLib {
template <typename T>
class OptionalStorage {
    alignas(T) uint8_t m_memory[sizeof(T)];
    bool m_exists{ false };
    public:
    OptionalStorage() = default;
    OptionalStorage(const OptionalStorage& other) {
        if (other.exists()) initialize(T{ other.as() });
    }
    OptionalStorage(OptionalStorage&& other) noexcept {
        if (other.exists()) initialize(move(other.as()));
    }
    OptionalStorage& operator=(const OptionalStorage& other) {
        destroy();
        if (other.exists()) { initialize(T{ other.as() }); }
        return *this;
    }
    OptionalStorage& operator=(OptionalStorage&& other) noexcept {
        destroy();
        if (other.exists()) { initialize(move(other.as())); }
        return *this;
    }
    void initialize(T&& value) {
        new (m_memory) T{ Forward<T>(value) };
        m_exists = true;
    }
    const T& as() const { return *reinterpret_cast<const T*>(m_memory); }
    T& as() { return *reinterpret_cast<T*>(m_memory); }
    const T* ptr() const { return reinterpret_cast<const T*>(m_memory); }
    T* ptr() { return reinterpret_cast<T*>(m_memory); }
    bool exists() const { return m_exists; }
    void destroy() {
        if (exists()) as().~T();
        m_exists = false;
    }
    T extract() {
        m_exists = false;
        return move(as());
    }
    ~OptionalStorage() { destroy(); }
};
template <typename T>
class OptionalStorage<T&> {
    T* m_value{ nullptr };
    public:
    OptionalStorage() = default;
    OptionalStorage(const OptionalStorage& other) { m_value = other.m_value; }
    OptionalStorage(OptionalStorage&& other) noexcept {
        m_value       = other.m_value;
        other.m_value = nullptr;
    }
    OptionalStorage& operator=(const OptionalStorage& other) {
        m_value = other.m_value;
        return *this;
    }
    OptionalStorage& operator=(OptionalStorage&& other) noexcept {
        m_value       = other.m_value;
        other.m_value = nullptr;
        return *this;
    }
    void initialize(T& value) { m_value = &value; }
    const T& as() const { return *m_value; }
    T& as() { return *m_value; }
    const T* ptr() const { return m_value; }
    T* ptr() { return m_value; }
    bool exists() const { return m_value != nullptr; }
    void destroy() { m_value = nullptr; }
    T& extract() {
        T& ret  = *m_value;
        m_value = nullptr;
        return ret;
    }
    ~OptionalStorage() { destroy(); }
};
template <typename T>
class Optional {
    OptionalStorage<T> m_object;

    using Ret   = ConditionalT<IsLvalueReferenceV<T>, T, AddLvalueReferenceT<T>>;
    using CTR   = AddConstT<AddLvalueReferenceT<RemoveCvRefT<T>>>;
    using BaseT = RemoveCvRefT<T>;

    public:
    Optional()                                 = default;
    Optional(const Optional&)                  = default;
    Optional(Optional&&)                       = default;
    Optional& operator=(const Optional& other) = default;
    Optional& operator=(Optional&& other)      = default;
    Optional(T&& val)
    requires(MoveAssignable<T> && !IsLvalueReferenceV<T>)
    {
        m_object.initialize(Forward<T>(val));
    }
    Optional(const T& val)
    requires(CopyAssignable<T> && !IsLvalueReferenceV<T>)
    {
        m_object.initialize(T{ val });
    }
    Optional(T val)
    requires IsLvalueReferenceV<T>
    {
        m_object.initialize(val);
    }
    Optional& operator=(const T& val)
    requires(CopyAssignable<T> && !IsLvalueReferenceV<T>)
    {
        m_object.destroy();
        m_object.initialize(T{ val });
        return *this;
    }
    Optional& operator=(T&& val)
    requires(MoveAssignable<T> && !IsLvalueReferenceV<T>)
    {
        m_object.destroy();
        m_object.initialize(Forward<T>(val));
        return *this;
    }
    Optional& operator=(T val)
    requires IsLvalueReferenceV<T>
    {
        m_object.destroy();
        m_object.initialize(val);
        return *this;
    }
    bool operator==(const Optional& other) const
    requires EqualityComparable<T>
    {
        // both are empty => true
        if (!m_object.exists() && !other.m_object.exists()) return true;
        // one of them is empty => false
        if (!m_object.exists() || !other.m_object.exists()) return false;
        return m_object.as() == other.m_object.as();
    }
    bool operator==(CTR other) const
    requires EqualityComparable<BaseT>
    {
        if (!m_object.exists()) return false;
        return m_object.as() == other;
    }
    Ordering operator<=>(const Optional& other) const {
        // both are empty => true
        if (!m_object.exists() && !other.m_object.exists()) return equal;
        // one of them is empty => false
        if (!m_object.exists()) return less;
        if (!other.m_object.exists()) return greater;
        if constexpr (Orderable<T>) {
            return value() <=> other.value();
        } else {
            return CompareThreeWay(value(), other.value());
        }
    }
    bool operator!() const { return !m_object.exists(); }
    explicit operator bool() const { return m_object.exists(); }
    bool empty() const { return !m_object.exists(); }
    bool has_value() const { return m_object.exists(); };
    AddConstT<Ret> value() const& { return m_object.as(); }
    Ret value() & { return m_object.as(); }
    T value() && { return m_object.extract(); }
    const auto* operator->() const { return m_object.ptr(); }
    auto* operator->() { return m_object.ptr(); }
    AddConstT<Ret> operator*() const { return m_object.as(); }
    Ret operator*() { return m_object.as(); }
    T value_or(T&& default_value) &&
    requires(!IsLvalueReferenceV<T>)
    {
        if (!m_object.exists()) return move(default_value);
        return m_object.extract();
    }
    T value_or(T default_value) &&
    requires IsLvalueReferenceV<T>
    {
        if (!m_object.exists()) return default_value;
        return m_object.extract();
    }
    template <typename... Args>
    requires(!IsLvalueReferenceV<T>)
    void emplace(Args... args) {
        m_object.destroy();
        m_object.initialize(T{ Forward<Args>(args)... });
    }
    void put(T&& value)
    requires(!IsLvalueReferenceV<T>)
    {
        m_object.destroy();
        m_object.initialize(T{ Forward<T>(value) });
    }
    void put(T value)
    requires IsLvalueReferenceV<T>
    {
        m_object.destroy();
        m_object.initialize(value);
    }
    T extract() { return m_object.extract(); }
    template <typename Func>
    Optional<InvokeResultT<Func, T>> map(Func func) & {
        if (!m_object.exists()) {
            return {};
        } else {
            return Optional{ invoke(func, m_object.as()) };
        }
    }
    template <typename Func>
    Optional<InvokeResultT<Func, T>> map(Func func) && {
        using OutT = Optional<InvokeResultT<Func, T>>;
        if (!m_object.exists()) {
            return OutT{};
        } else {
            return OutT{ invoke(func, move(m_object.extract())) };
        }
    }
    void evict() { m_object.destroy(); }
    ~Optional() { evict(); }
};
template <typename T>
struct Hash<Optional<T>> {
    constexpr static inline auto empty_opt_hash = static_cast<size_t>(-1);
    [[nodiscard]] size_t operator()(const Optional<T>& key) const noexcept {
        return key.has_value() ? Hash<T>{}(key.value()) : empty_opt_hash;
    }
};
template <typename T>
struct Hash<Optional<T&>> {
    constexpr static inline auto empty_opt_hash = static_cast<size_t>(-1);
    [[nodiscard]] size_t operator()(const Optional<T&>& key) const noexcept {
        return key.has_value() ? Hash<T>{}(key.value()) : empty_opt_hash;
    }
};
template <Printable T>
struct PrintInfo<Optional<T>> {
    const Optional<T>& m_optional;
    explicit PrintInfo(const Optional<T>& optional) : m_optional(optional) {}
    String repr() const {
        if (m_optional.empty()) {
            return "Empty optional"_s;
        } else {
            return "Optional { "_s + PrintInfo<T>{ m_optional.value() }.repr() + " }"_s;
        }
    }
};
template <Printable T>
struct PrintInfo<Optional<T&>> {
    const Optional<T&>& m_optional;
    explicit PrintInfo(const Optional<T&>& optional) : m_optional(optional) {}
    String repr() const {
        if (m_optional.empty()) {
            return "Empty optional reference"_s;
        } else {
            return "OptionalRef { "_s + PrintInfo<T>{ m_optional.value() }.repr() + " }"_s;
        }
    }
};
}    // namespace ARLib
