#pragma once
// minimal set of includes to make the library work
#include <new>
#ifdef COMPILER_MSVC
    #include <initializer_list>
#else
    // a little bit of undefined behavior never hurt anyone
    #ifndef _INITIALIZER_LIST
        #define _INITIALIZER_LIST
namespace std {
template <class E>
class initializer_list {
    public:
    typedef E value_type;
    typedef const E& reference;
    typedef const E& const_reference;
    typedef size_t size_type;
    typedef const E* iterator;
    typedef const E* const_iterator;

    private:
    iterator m_array;
    size_type m_len;
    constexpr initializer_list(const_iterator a, size_type l) : m_array(a), m_len(l) {}

    public:
    constexpr initializer_list() noexcept : m_array(0), m_len(0) {}
    constexpr size_type size() const noexcept { return m_len; }
    constexpr const_iterator begin() const noexcept { return m_array; }
    constexpr const_iterator end() const noexcept { return begin() + size(); }
};
template <class E>
constexpr const E* begin(initializer_list<E> ils) noexcept {
    return ils.begin();
}
template <class E>
constexpr const E* end(initializer_list<E> ils) noexcept {
    return ils.end();
}
}    // namespace std
    #endif
#endif
