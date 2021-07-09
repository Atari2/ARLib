#pragma once
#include "Vector.h"

namespace ARLib {

    template <typename T>
    class Stack {
        Vector<T> m_stack;

        public:
        Stack() = default;
        void push(T value) { m_stack.push_back(Forward<T>(value)); }
        T pop() { return m_stack.pop(); }
        const T& peek() const { return m_stack[m_stack.size() - 1]; }
        size_t size() const { return m_stack.size(); }
        T& peek() { return m_stack[m_stack.size() - 1]; }
    };

} // namespace ARLib