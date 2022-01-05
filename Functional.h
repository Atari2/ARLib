#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "Invoke.h"
#include "PrintInfo.h"
#include "TypeTraits.h"
#include "Utility.h"

namespace ARLib {
    namespace fntraits {
        template <typename Arg, typename Result>
        struct UnaryFn {
            typedef Arg argtype;
            typedef Result restype;
        };

        template <typename Arg1, typename Arg2, typename Result>
        struct BinaryFn {
            typedef Arg1 firstargtype;
            typedef Arg2 secondargtype;
            typedef Result restype;
        };

        template <typename Res, typename... Args>
        struct MbUOrBFn {};

        template <typename Res, typename T1>
        struct MbUOrBFn<Res, T1> : UnaryFn<T1, Res> {};

        template <typename Res, typename T1, typename T2>
        struct MbUOrBFn<Res, T1, T2> : BinaryFn<T1, T2, Res> {};
        class UndefinedClass;

        template <typename T>
        struct IsLocationInvariant : IsTriviallyCopiable<T>::type {};

        union NocopyTypes {
            void* m_object = nullptr;
            const void* m_const_object;
            void (*m_fn_ptr)();
            void (UndefinedClass::*m_mbfn_ptr)();
        };

        union AnyData {
            void* m_access() { return &m_pod_data[0]; }
            const void* m_access() const { return &m_pod_data[0]; }

            template <typename T>
            T& m_access() {
                return *static_cast<T*>(m_access());
            }

            template <typename T>
            const T& m_access() const {
                return *static_cast<const T*>(m_access());
            }

            NocopyTypes m_unused;
            char m_pod_data[sizeof(NocopyTypes)]{0};
        };

        enum class ManagerOp { GetFuncPtr, CloneFunc, DestroyFunc };
    } // namespace fntraits
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

        const auto& partial_args() const { return m_pargs; }
        constexpr explicit PartialFunction(Func func, Args... args) : m_function(move(func)) {
            MakeIndexSequence<sizeof...(Args)> seq{};
            apply_impl(Forward<Args>(args)..., seq);
        }

        template <typename... Rest>
        constexpr ResultOfT<Func(Args..., Rest...)>
        operator()(Rest... rest) requires CallableWith<Func, Args..., Rest...> {
            return run_impl<sizeof...(Args) - 1>(rest...);
        }
    };

    template <typename Sig>
    class Function;

    class FunctionBase {
        public:
        static constexpr size_t m_max_size = sizeof(fntraits::NocopyTypes);
        static constexpr size_t m_max_align = alignof(fntraits::NocopyTypes);

        template <typename Functor>
        class BaseManager {
            protected:
            static const bool m_stored_locally =
            (fntraits::IsLocationInvariant<Functor>::value && sizeof(Functor) <= m_max_size &&
             alignof(Functor) <= m_max_align && (m_max_align % alignof(Functor) == 0));

            typedef IntegralConstant<bool, m_stored_locally> m_local_storage;

            static Functor* m_get_ptr(const fntraits::AnyData& source) {
                if constexpr (m_stored_locally) {
                    const Functor& f = source.m_access<Functor>();
                    return const_cast<Functor*>(addressof(f));
                } else // have stored a pointer
                    return source.m_access<Functor*>();
            }

            static void m_clone(fntraits::AnyData& dest, const fntraits::AnyData& source, TrueType) {
                ::new (dest.m_access()) Functor(source.m_access<Functor>());
            }

            static void m_clone(fntraits::AnyData& dest, const fntraits::AnyData& source, FalseType) {
                dest.m_access<Functor*>() = new Functor(*source.m_access<const Functor*>());
            }

            static void m_destroy(fntraits::AnyData& victim, TrueType) { victim.m_access<Functor>().~Functor(); }

            static void m_destroy(fntraits::AnyData& victim, FalseType) { delete victim.m_access<Functor*>(); }

            public:
            static bool m_manager(fntraits::AnyData& dest, const fntraits::AnyData& source, fntraits::ManagerOp op) {
                switch (op) {
                case fntraits::ManagerOp::GetFuncPtr:
                    dest.m_access<Functor*>() = m_get_ptr(source);
                    break;

                case fntraits::ManagerOp::CloneFunc:
                    m_clone(dest, source, m_local_storage());
                    break;

                case fntraits::ManagerOp::DestroyFunc:
                    m_destroy(dest, m_local_storage());
                    break;
                }
                return false;
            }

            static void m_init_functor(fntraits::AnyData& functor, Functor&& f) {
                m_init_functor(functor, move(f), m_local_storage());
            }

            template <typename Signature>
            static bool m_not_empty_function(const Function<Signature>& f) {
                return static_cast<bool>(f);
            }

            template <typename T>
            static bool m_not_empty_function(T* fp) {
                return fp != nullptr;
            }

            template <typename Class, typename T>
            static bool m_not_empty_function(T Class::*mp) {
                return mp != nullptr;
            }

            template <typename T>
            static bool m_not_empty_function(const T&) {
                return true;
            }

            private:
            static void m_init_functor(fntraits::AnyData& func, Functor&& f, TrueType) {
                ::new (func.m_access()) Functor(move(f));
            }

            static void m_init_functor(fntraits::AnyData& func, Functor&& f, FalseType) {
                func.m_access<Functor*>() = new Functor(move(f));
            }
        };

        FunctionBase() : m_functor{}, m_manager(nullptr) {}

        ~FunctionBase() {
            if (m_manager) m_manager(m_functor, m_functor, fntraits::ManagerOp::DestroyFunc);
        }

        bool m_empty() const { return !m_manager; }

        typedef bool (*ManagerType)(fntraits::AnyData&, const fntraits::AnyData&, fntraits::ManagerOp);

        fntraits::AnyData m_functor;
        ManagerType m_manager;
    };

    template <typename Signature, typename Functor>
    class FunctionHandler;

    template <typename Res, typename Functor, typename... Args>
    class FunctionHandler<Res(Args...), Functor> : public FunctionBase::BaseManager<Functor> {
        typedef FunctionBase::BaseManager<Functor> Base;

        public:
        static bool m_manager(fntraits::AnyData& dest, const fntraits::AnyData& source, fntraits::ManagerOp op) {
            switch (op) {
            case fntraits::ManagerOp::GetFuncPtr:
                dest.m_access<Functor*>() = Base::m_get_ptr(source);
                break;

            default:
                Base::m_manager(dest, source, op);
            }
            return false;
        }

        static Res m_invoke(const fntraits::AnyData& functor, Args&&... args) {
            return invoke_r<Res>(*Base::m_get_ptr(functor), Forward<Args>(args)...);
        }
    };

    template <>
    class FunctionHandler<void, void> {
        public:
        static bool m_manager(fntraits::AnyData&, const fntraits::AnyData&, fntraits::ManagerOp) { return false; }
    };

    template <typename Signature, typename Functor, bool valid = IsObject<Functor>::value>
    struct TargetHandler : FunctionHandler<Signature, typename RemoveCv<Functor>::type> {};

    template <typename Signature, typename Functor>
    struct TargetHandler<Signature, Functor, false> : FunctionHandler<void, void> {};

    template <typename Res, typename... Args>
    class Function<Res(Args...)> : public fntraits::MbUOrBFn<Res, Args...>, private FunctionBase {
        template <typename Func, typename Res2 = InvokeResult<Func&, Args...>>
        struct Callable : IsInvokableImpl<Res2, Res>::type {};

        template <typename T>
        struct Callable<Function, T> : FalseType {};

        template <typename Cond, typename T>
        using Requires = typename EnableIf<Cond::value, T>::type;

        public:
        typedef Res result_type;

        Function() noexcept : FunctionBase() {}

        Function(nullptr_t) noexcept : FunctionBase() {}

        Function(const Function& x) : FunctionBase() {
            if (static_cast<bool>(x)) {
                x.m_manager(m_functor, x.m_functor, fntraits::ManagerOp::CloneFunc);
                m_invoker = x.m_invoker;
                m_manager = x.m_manager;
            }
        }

        Function(Function&& x) noexcept : FunctionBase() { x.swap(*this); }

        template <typename Functor, typename = Requires<Not<IsSame<Functor, Function>>, void>,
                  typename = Requires<Callable<Functor>, void>>
        Function(Functor f) : FunctionBase() {
            typedef FunctionHandler<Res(Args...), Functor> my_handler;

            if (my_handler::m_not_empty_function(f)) {
                my_handler::m_init_functor(m_functor, move(f));
                m_invoker = &my_handler::m_invoke;
                m_manager = &my_handler::m_manager;
            }
        }

        Function& operator=(const Function& x) {
            Function(x).swap(*this);
            return *this;
        }

        Function& operator=(Function&& x) noexcept {
            Function(move(x)).swap(*this);
            return *this;
        }

        Function& operator=(nullptr_t) noexcept {
            if (m_manager) {
                m_manager(m_functor, m_functor, fntraits::ManagerOp::DestroyFunc);
                m_manager = nullptr;
                m_invoker = nullptr;
            }
            return *this;
        }

        template <typename Functor>
        Requires<Callable<typename Decay<Functor>::type>, Function&> operator=(Functor&& f) {
            Function(Forward<Functor>(f)).swap(*this);
            return *this;
        }

        template <typename Functor>
        Function& operator=(detail::ReferenceWrapper<Functor> f) noexcept {
            Function(f).swap(*this);
            return *this;
        }

        void swap(Function& x) noexcept {
            ARLib::swap(m_functor, x.m_functor);
            ARLib::swap(m_manager, x.m_manager);
            ARLib::swap(m_invoker, x.m_invoker);
        }

        explicit operator bool() const noexcept { return !m_empty(); }

        Res operator()(Args... args) const {
            HARD_ASSERT(!m_empty(), "Function can't be empty on call")
            return m_invoker(m_functor, Forward<Args>(args)...);
        }

        private:
        using InvokerType = Res (*)(const fntraits::AnyData&, Args&&...);
        InvokerType m_invoker = nullptr;
    };

    template <typename Arg, typename... Args>
    struct PrintInfo<detail::PartialArguments<Arg, Args...>> {
        const detail::PartialArguments<Arg, Args...>& m_partial;
        explicit PrintInfo(const detail::PartialArguments<Arg, Args...>& partial) : m_partial(partial) {}
        String repr() {
            if constexpr (sizeof...(Args) == 0) {
                return String{typeid(Arg).name()};
            } else {
                using Inner = detail::PartialArguments<Args...>;
                return String{typeid(Arg).name()} + ", "_s +
                       PrintInfo<Inner>{static_cast<const Inner&>(m_partial)}.repr();
            }
        }
    };

    template <typename Func, typename... Args>
    struct PrintInfo<PartialFunction<Func, Args...>> {
        const PartialFunction<Func, Args...>& m_func;
        explicit PrintInfo(const PartialFunction<Func, Args...>& func) : m_func(func) {}
        String repr() {
            return "PartialFunction "_s + String{typeid(Func).name()} + " with partial arguments: ("_s +
                   PrintInfo<detail::PartialArguments<Args...>>{m_func.partial_args()}.repr() + ")"_s;
        }
    };
} // namespace ARLib