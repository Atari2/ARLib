#pragma once
#include "Assertion.hpp"
#include "Concepts.hpp"
#include "HashBase.hpp"
#include "Ordering.hpp"
#include "PrintInfo.hpp"
namespace ARLib {
template <typename T>
class Optional {
    T* m_object   = nullptr;
    bool m_exists = false;
    void assert_not_null_() const { HARD_ASSERT(m_object, "Null deref in Optional") }
    void evict_() {
        if (m_exists) delete m_object;
        m_exists = false;
    }

    public:
    Optional() = default;
    Optional(const Optional& other) {
        if (other.m_exists) { m_object = new T{ *other.m_object }; }
        m_exists = other.m_exists;
    }
    Optional(Optional&& other) noexcept {
        m_object       = other.m_object;
        m_exists       = other.m_exists;
        other.m_object = nullptr;
        other.m_exists = false;
    }
    Optional& operator=(const Optional& other) {
        if (this == &other) return *this;
        evict_();
        if (other.m_exists) *m_object = *other.m_object;
        m_exists = other.m_exists;
        return *this;
    }
    Optional& operator=(Optional&& other) noexcept {
        evict_();
        m_object       = other.m_object;
        m_exists       = other.m_exists;
        other.m_object = nullptr;
        other.m_exists = false;
        return *this;
    }
    Optional(T&& val)
    requires MoveAssignable<T>
        : m_object(new T{ move(val) }), m_exists(true) {}
    Optional(const T& val)
    requires CopyAssignable<T>
        : m_object(new T{ val }), m_exists(true) {}
    Optional& operator=(const T& val)
    requires CopyAssignable<T>
    {
        evict_();
        m_object = new T{ val };
        m_exists = true;
        return *this;
    }
    Optional& operator=(T&& val)
    requires MoveAssignable<T>
    {
        evict_();
        m_object = new T{ Forward<T>(val) };
        m_exists = true;
        return *this;
    }
    bool operator==(const Optional& other) const
    requires EqualityComparable<T>
    {
        // both are empty => true
        if (!m_exists && !other.m_exists) return true;
        // one of them is empty => false
        if (!m_exists || !other.m_exists) return false;
        return *m_object == *other.m_object;
    }
    Ordering operator<=>(const Optional& other) const {
        // both are empty => true
        if (!m_exists && !other.m_exists) return equal;
        // one of them is empty => false
        if (!m_exists) return less;
        if (!other.m_exists) return greater;
        if constexpr (Orderable<T>) {
            return value() <=> other.value();
        } else {
            return CompareThreeWay(value(), other.value());
        }
    }
    bool operator!() const { return !m_exists; }
    explicit operator bool() const { return m_exists; }
    bool empty() const { return !m_exists; }
    bool has_value() const { return m_exists; };
    const T& value() const& {
        assert_not_null_();
        return *m_object;
    }
    T& value() & {
        assert_not_null_();
        return *m_object;
    }
    T value() && {
        assert_not_null_();
        T val = move(*m_object);
        evict_();
        return val;
    }
    const T* operator->() const {
        assert_not_null_();
        return m_object;
    }
    T* operator->() {
        assert_not_null_();
        return m_object;
    }
    const T& operator*() const {
        assert_not_null_();
        return *m_object;
    }
    T& operator*() {
        assert_not_null_();
        return *m_object;
    }
    T value_or(T&& default_value) && {
        if (!m_exists) return move(default_value);
        T val = move(*m_object);
        evict_();
        return val;
    }
    template <typename... Args>
    void emplace(Args... args) {
        m_object = new T(Forward<Args>(args)...);
        m_exists = true;
    }
    void put(T&& value) {
        if (m_exists) evict_();
        m_object = new T{ Forward<T>(value) };
        m_exists = true;
    }
    T extract() {
        assert_not_null_();
        T val = move(*m_object);
        evict_();
        return move(val);
    }
    T* detach() {
        assert_not_null_();
        T* val   = m_object;
        m_object = nullptr;
        m_exists = false;
        return val;
    }
    void evict() { evict_(); }
    ~Optional() { evict_(); }
};
template <typename T>
struct Hash<Optional<T>> {
    constexpr static inline auto empty_opt_hash = static_cast<size_t>(-1);
    [[nodiscard]] size_t operator()(const Optional<T>& key) const noexcept {
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
}    // namespace ARLib
