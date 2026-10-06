#pragma once
#include "Utility.hpp"
#include "Variant.hpp"
#include "SharedPtr.hpp"
#include "RefBox.hpp"
namespace ARLib {
template <typename T>
class MaybeOwned {
    Variant<SharedPtr<T>, RefBox<T>> m_obj;

    T* internal_ptr() {
        if (m_obj.template contains_type<SharedPtr<T>>()) {
            return m_obj.template get<SharedPtr<T>>().get();
        } else {
            return m_obj.template get<RefBox<T>>().ptr();
        }
    }
    const T* internal_ptr() const {
        if (m_obj.template contains_type<SharedPtr<T>>()) {
            return m_obj.template get<SharedPtr<T>>().get();
        } else {
            return m_obj.template get<RefBox<T>>().ptr();
        }
    }
    MaybeOwned(T* ptr) : m_obj{ SharedPtr<T>{ ptr } } {}
    MaybeOwned(T& ref) : m_obj{ RefBox<T>{ ref } } {}

    public:
    MaybeOwned(MaybeOwned&&)      = default;
    MaybeOwned(const MaybeOwned&) = default;
    template <DerivedFrom<T> U>
    static MaybeOwned owned(U&& object)
    requires MoveConstructible<U>
    {
        U* ptr = new U(Forward<U>(object));
        return MaybeOwned{ static_cast<T*>(ptr) };
    }
    static MaybeOwned owned(T&& object)
    requires MoveConstructible<T>
    {
        T* ptr = new T(Forward<T>(object));
        return MaybeOwned{ ptr };
    }
    template <DerivedFrom<T> U>
    static MaybeOwned lended(U& object) { return MaybeOwned{ static_cast<T&>(object) }; }
    static MaybeOwned lended(T& object) { return MaybeOwned{ object }; }
    T* ptr() { return internal_ptr(); }
    const T* ptr() const { return internal_ptr(); }
    T& operator*() { return *internal_ptr(); }
    const T& operator*() const { return *internal_ptr(); }
    T* operator->() { return internal_ptr(); }
    const T* operator->() const { return internal_ptr(); }
    bool operator==(const MaybeOwned& other) const { return m_obj == other.m_obj; }
    bool operator!=(const MaybeOwned& other) const { return m_obj != other.m_obj; }
};
}    // namespace ARLib