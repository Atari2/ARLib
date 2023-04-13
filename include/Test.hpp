#pragma once
#include "Assertion.hpp"
#include "Concepts.hpp"
#include "SourceLocation.hpp"
namespace ARLib {
template <typename Fn, typename... Args>
requires SameAs<ResultOfT<Fn(Args...)>, bool>
class Test {
    Fn m_func;

    public:
    explicit Test(Fn func) : m_func(func) {}
    bool run(Args... args) { return m_func(args...); }
};
}    // namespace ARLib
