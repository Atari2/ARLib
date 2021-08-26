#pragma once
#include "Concepts.h"
#include "Utility.h"


//TODO: make sure to overload everything correctly for rvalue and lvalue references

namespace ARLib {
    template <typename T, typename... Args>
    class Tuple : Tuple<Args...> {
        T m_member;

        template <size_t N>
        bool equality_impl(const Tuple& other) const {
            if constexpr (N == 0)
                return m_member == other.m_member;
            else {
                if (this->template get<N>() == other.template get<N>()) {
                    return equality_impl<N - 1>(other);
                } else {
                    return false;
                }
            }
        }

        public:
        Tuple() = default;
        Tuple(const Tuple&) = default;
        Tuple(Tuple&&) noexcept = default;

        explicit Tuple(T arg, Args... args) : Tuple<Args...>(args...), m_member(move(arg)) {}
        Tuple& operator=(const Tuple&) = default;
        Tuple& operator=(Tuple&&) noexcept = default;

        bool operator==(const Tuple& other) const { return equality_impl<sizeof...(Args) - 1>(other); }

        Tuple& operator=(T&& value) {
            m_member = move(value);
            return *this;
        }

        template <typename U>
        requires IsAnyOfV<U, Args...> Tuple& operator=(U&& value) {
            static_cast<Tuple<Args...>*>(this)->template set<U>(Forward<U>(value));
            return *this;
        }

        template <typename Tp>
        constexpr const Tp& get() const {
            static_assert(IsAnyOfV<Tp, T, Args...>);
            if constexpr (SameAs<Tp, T>) {
                return m_member;
            } else {
                return static_cast<const Tuple<Args...>*>(this)->template get<Tp>();
            }
        }

        template <typename Tp>
        constexpr Tp& get() {
            static_assert(IsAnyOfV<Tp, T, Args...>);
            if constexpr (SameAs<Tp, T>) {
                return m_member;
            } else {
                return static_cast<Tuple<Args...>*>(this)->template get<Tp>();
            }
        }

        template <size_t N>
        constexpr const auto& get() const {
            if constexpr (N == 0) {
                return m_member;
            } else {
                static_assert(N < (sizeof...(Args) + 1) && N > 0);
                return static_cast<const Tuple<Args...>*>(this)->template get<N - 1>();
            }
        }

        template <size_t N>
        constexpr auto& get() {
            if constexpr (N == 0) {
                return m_member;
            } else {
                static_assert(N < (sizeof...(Args) + 1) && N > 0);
                return static_cast<Tuple<Args...>*>(this)->template get<N - 1>();
            }
        }

        template <typename Tp>
        constexpr void set(Tp&& p) {
            static_assert(IsAnyOfV<Tp, T, Args...>);
            if constexpr (SameAs<Tp, T>) {
                m_member = move(p);
            } else {
                static_cast<Tuple<Args...>*>(this)->template set<Tp>(Forward<Tp>(p));
            }
        }

        template <size_t N, typename F>
        constexpr void set(F&& f) {
            if constexpr (N == 0) {
                m_member = move(f);
            } else {
                static_assert(N < (sizeof...(Args) + 1) && N > 0);
                static_cast<Tuple<Args...>*>(this)->template set<N - 1, F>(Forward<F>(f));
            }
        }
    };

    template <typename T>
    class Tuple<T> {
        T m_member;

        public:
        Tuple() = default;
        explicit Tuple(T arg) : m_member(move(arg)) {}

        bool operator==(const Tuple& other) const { return m_member == other.m_member; }

        Tuple& operator=(T&& value) {
            m_member = move(value);
            return *this;
        }

        template <typename Tp>
        T& get() {
            static_assert(SameAs<Tp, T>);
            return m_member;
        }

        template <typename Tp>
        const T& get() const {
            static_assert(SameAs<Tp, T>);
            return m_member;
        }

        template <size_t N>
        constexpr T& get() {
            static_assert(N == 0);
            return m_member;
        }

        template <size_t N>
        constexpr const T& get() const {
            static_assert(N == 0);
            return m_member;
        }

        template <typename Tp>
        void set(Tp&& p) {
            static_assert(SameAs<Tp, T>);
            m_member = move(p);
        }

        template <size_t N, typename _>
        constexpr void set(T&& f) {
            static_assert(N == 0);
            m_member = move(f);
        }
    };
} // namespace ARLib