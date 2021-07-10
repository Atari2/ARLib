#pragma once
#include "Utility.h"

namespace ARLib {
    template <typename T, typename... Args>
    class FuncMember : FuncMember<Args...> {
        T m_mem;

        public:
        template <size_t N>
        constexpr auto& get() {
            if constexpr (N == 0) {
                return m_mem;
            } else {
                static_assert(N < (sizeof...(Args) + 1) && N > 0);
                return static_cast<FuncMember<Args...>*>(this)->template get<N - 1>();
            }
        }

        template <size_t N, typename F>
        constexpr void set(F&& f) {
            if constexpr (N == 0) {
                m_mem = move(f);
            } else {
                static_assert(N < (sizeof...(Args) + 1) && N > 0);
                static_cast<FuncMember<Args...>*>(this)->template set<N - 1, F>(Forward<F>(f));
            }
        }
    };

    template <typename T>
    class FuncMember<T> {
        T m_mem;

        public:
        template <size_t N>
        constexpr T& get() {
            static_assert(N == 0);
            return m_mem;
        }

        template <size_t N, typename Placeholder>
        constexpr void set(T&& f) {
            static_assert(N == 0);
            m_mem = move(f);
        }
    };

    template <typename Func, typename... Args>
    class Function {
        FuncMember<Args...> m_args;

        public:
        constexpr Function(Func func) : m_func(move(func)) {}

        constexpr auto run() { return run_impl<0>(); }

        template <size_t N, typename... Partials>
        constexpr auto run_impl(Partials&... pargs) {
            if constexpr (N == sizeof...(Args))
                return m_func(pargs...);
            else
                return run_impl<N + 1>(pargs..., m_args.template get<N>());
        }

        constexpr void apply(Args&&... args) {
            MakeIndexSequence<sizeof...(Args)> seq{};
            apply_impl(Forward<Args>(args)..., seq);
        }

        template <size_t... Indexes>
        constexpr void apply_impl(Args&&... args, IndexSequence<Indexes...>) {
            (m_args.template set<Indexes>(Forward<Args>(args)), ...);
        }

        constexpr auto operator()() { return run(); }

        constexpr auto operator()(Args&&... args) {
            apply(Forward<Args>(args)...);
            return run();
        }

        private:
        Func m_func;
    };

} // namespace ARLib