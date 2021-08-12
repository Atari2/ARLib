#pragma once
#include "Concepts.h"
#include "TypeTraits.h"
#include "Utility.h"

namespace ARLib {

    namespace detail {
        template <typename T, typename... Args>
        class PartialArguments : PartialArguments<Args...> {
            T m_arg;
            constexpr static auto NARGS = sizeof...(Args) + 1;

            public:
            PartialArguments() = default;
            PartialArguments(const PartialArguments&) = default;
            PartialArguments(PartialArguments&&) noexcept = default;
            PartialArguments& operator=(const PartialArguments&) = default;
            PartialArguments& operator=(PartialArguments&&) noexcept = default;

            template <size_t N>
            constexpr auto& get() {
                if constexpr (N == 0) {
                    return m_arg;
                } else {
                    static_assert(N < NARGS && N > 0);
                    return static_cast<PartialArguments<Args...>*>(this)->template get<N - 1>();
                }
            }

            template <size_t N, typename F>
            constexpr void set(F&& f) {
                if constexpr (N == 0) {
                    m_arg = move(f);
                } else {
                    static_assert(N < NARGS && N > 0);
                    static_cast<PartialArguments<Args...>*>(this)->template set<N - 1, F>(Forward<F>(f));
                }
            }
        };
        template <typename T>
        class PartialArguments<T> {
            T m_arg;

            public:
            PartialArguments() = default;
            PartialArguments(const PartialArguments&) = default;
            PartialArguments(PartialArguments&&) noexcept = default;
            PartialArguments& operator=(const PartialArguments&) = default;
            PartialArguments& operator=(PartialArguments&&) noexcept = default;

            template <size_t N>
            constexpr T& get() {
                static_assert(N == 0);
                return m_arg;
            }

            template <size_t N, typename = VoidT<>>
            constexpr void set(T&& f) {
                static_assert(N == 0);
                m_arg = move(f);
            }
        };
    } // namespace detail

    template <typename Func, typename... Args>
    class PartialFunction {
        Func m_function;
        detail::PartialArguments<Args...> m_pargs;

        template <size_t... Indexes>
        constexpr void apply_impl(Args&&... args, IndexSequence<Indexes...>) {
            (m_pargs.template set<Indexes>(Forward<Args>(args)), ...);
        }

        template <size_t N, typename... Rest>
        constexpr auto run_impl(Rest&... rest) {
            if constexpr (N == 0)
                return m_function(m_pargs.template get<0>(), rest...);
            else
                return run_impl<N - 1>(m_pargs.template get<N>(), rest...);
        }

        public:
        PartialFunction() = default;
        PartialFunction(const PartialFunction&) = default;
        PartialFunction(PartialFunction&&) noexcept = default;
        PartialFunction& operator=(const PartialFunction&) = default;
        PartialFunction& operator=(PartialFunction&&) noexcept = default;
        constexpr PartialFunction(Func func, Args... args) : m_function(move(func)) {
            MakeIndexSequence<sizeof...(Args)> seq{};
            apply_impl(Forward<Args>(args)..., seq);
        }

        const auto& partial_args() const { return m_pargs; }

        template <typename... Rest>
        constexpr ResultOfT<Func(Args..., Rest...)>
        operator()(Rest... rest) requires CallableWith<Func, Args..., Rest...> {
            return run_impl<sizeof...(Args) - 1>(rest...);
        }
    };
} // namespace ARLib