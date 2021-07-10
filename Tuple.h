#pragma once
#include "Concepts.h"
#include "Utility.h"

namespace ARLib {
    template <typename T, typename... Args>
    class Tuple : Tuple<Args...> {
        T m_member;

        public:
        Tuple() = default;
        Tuple(const Tuple&) = default;
        Tuple(Tuple&&) = default;

        Tuple(T arg, Args... args) : Tuple<Args...>(args...), m_member(move(arg)) {}
        Tuple& operator=(const Tuple&) = default;
        Tuple& operator=(Tuple&&) = default;

        template <typename Tp>
        constexpr Tp& get_typed() {
            static_assert(IsAnyOfV<Tp, T, Args...>);
            if constexpr (SameAs<Tp, T>) {
                return m_member;
            } else {
                return static_cast<Tuple<Args...>*>(this)->template get_typed<Tp>();
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
        constexpr void set_typed(Tp&& p) {
            static_assert(IsAnyOfV<Tp, T, Args...>);
            if constexpr (SameAs<Tp, T>) {
                m_member = move(p);
            } else {
                static_cast<Tuple<Args...>*>(this)->template set_typed<Tp>(Forward<Tp>(p));
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
        Tuple(T arg) : m_member(move(arg)) {}

        template <typename Tp>
        T& get_typed() {
            static_assert(SameAs<Tp, T>);
            return m_member;
        }

        template <size_t N>
        constexpr T& get() {
            static_assert(N == 0);
            return m_member;
        }

        template <typename Tp>
        void set_typed(Tp&& p) {
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