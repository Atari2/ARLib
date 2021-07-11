#pragma once
#include "Concepts.h"
#include "Utility.h"

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
        Tuple(Tuple&&) = default;

        Tuple(T arg, Args... args) : Tuple<Args...>(args...), m_member(move(arg)) {}
        Tuple& operator=(const Tuple&) = default;
        Tuple& operator=(Tuple&&) = default;

        bool operator==(const Tuple& other) const { return equality_impl<sizeof...(Args) - 1>(other); }

        template <typename Tp>
        constexpr const Tp& get_typed() const {
            static_assert(IsAnyOfV<Tp, T, Args...>);
            if constexpr (SameAs<Tp, T>) {
                return m_member;
            } else {
                return static_cast<const Tuple<Args...>*>(this)->template get_typed<Tp>();
            }
        }

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

        bool operator==(const Tuple& other) const { return m_member == other.m_member; }

        template <typename Tp>
        T& get_typed() {
            static_assert(SameAs<Tp, T>);
            return m_member;
        }

        template <typename Tp>
        const T& get_typed() const {
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