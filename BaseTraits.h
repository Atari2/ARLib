#pragma once
#include "Types.h"
#include <type_traits>

namespace ARLib {
    // the overwhelming majority of the type traits are copy-pasted from msvc's implementation
    // since those are reliable and this is a finnicky thing
    // it just made sense to use those, so I can both have a correct and malleable implementation.
    // aliases to std:: members that I have no clue how to implement
    template <class T, class... Args>
    using ConstructibleImpl = std::is_constructible<T, Args...>;
    template <class T, class... Args>
    using TriviallyConstructibleImpl = std::is_trivially_constructible<T, Args...>;
    template <class T, class... Args>
    using NothrowConstructibleImpl = std::is_nothrow_constructible<T, Args...>;

    template <class T, class U>
    using AssignableImpl = std::is_assignable<T, U>;
    template <class T, class U>
    using TriviallyAssignableImpl = std::is_trivially_assignable<T, U>;
    template <class T, class U>
    using NothrowAssignableImpl = std::is_nothrow_assignable<T, U>;

#if not(defined(COMPILER_MSVC) || (defined(WINDOWS) && defined(COMPILER_CLANG)))
    template <class T>
    using IsDestructibleSafe = std::__is_destructible_safe<T>;
#endif

    template <class T>
    using IsUnion = std::is_union<T>;

    template <class T>
    using IsEnum = std::is_enum<T>;

    template <class T>
    inline constexpr bool IsEnumV = IsEnum<T>::value;

    template <typename...>
    using VoidT = void;

    // type traits

    // integral constant
    template <class T, T Val>
    struct IntegralConstant {
        static constexpr T value = Val;

        using value_type = T;
        using type = IntegralConstant;

        constexpr operator value_type() const noexcept { return value; }

        [[nodiscard]] constexpr value_type operator()() const noexcept { return value; }
    };

    template <bool V>
    using BoolConstant = IntegralConstant<bool, V>;

    using TrueType = BoolConstant<true>;
    using FalseType = BoolConstant<false>;

    // conditional
    template <bool cond, class TrueT, class FalseT>
    struct Conditional {
        using type = TrueT;
    };

    template <class TrueT, class FalseT>
    struct Conditional<false, TrueT, FalseT> {
        using type = FalseT;
    };

    template <bool cond, class TrueType, class FalseType>
    using ConditionalT = typename Conditional<cond, TrueType, FalseType>::type;

    namespace detail {
        template <typename...>
        struct And;

        template <>
        struct And<> : public TrueType {};

        template <typename B1>
        struct And<B1> : public B1 {};

        template <typename B1, typename B2>
        struct And<B1, B2> : public Conditional<B1::value, B2, B1>::type {};

        template <typename B1, typename B2, typename B3, typename... Bn>
        struct And<B1, B2, B3, Bn...> : public Conditional<B1::value, And<B2, B3, Bn...>, B1>::type {};
    } // namespace detail

    template <class T>
    struct IsTriviallyCopiable : BoolConstant<__is_trivially_copyable(T)> {};

#if defined(COMPILER_MSVC) || (defined(COMPILER_CLANG) && defined(WINDOWS))

    template <class T>
    struct IsTriviallyDestructible : BoolConstant<__is_trivially_destructible(T)> {};

    template <class T>
    inline constexpr bool IsTriviallyDestructibleV = __is_trivially_destructible(T);

#else

    template <class T>
    struct IsTriviallyDestructible : detail::And<IsDestructibleSafe<T>, BoolConstant<__has_trivial_destructor(T)>> {};

    template <class T>
    inline constexpr bool IsTriviallyDestructibleV = IsTriviallyDestructible<T>::value;

#endif

    template <class T>
    inline constexpr bool IsTriviallyCopiableV = __is_trivially_copyable(T);

    template <class T>
    struct IsTrivial : BoolConstant<__is_trivially_constructible(T) && __is_trivially_copyable(T)> {};

    template <class T>
    inline constexpr bool IsTrivialV = __is_trivially_constructible(T) && __is_trivially_copyable(T);

    template <class T>
    struct IsConst : FalseType {};
    template <class T>
    struct IsConst<const T> : TrueType {};

    template <class T>
    struct IsReference : FalseType {};
    template <class T>
    struct IsReference<T&> : TrueType {};
    template <class T>
    struct IsReference<T&&> : TrueType {};

    template <class T>
    struct IsArray : FalseType {};

    template <class T>
    struct IsArray<T[]> : TrueType {};

    template <class T, size_t N>
    struct IsArray<T[N]> : TrueType {};

    template <class T>
    struct IsFunction : IntegralConstant<bool, !IsConst<const T>::value && !IsReference<T>::value> {};

    template <class T>
    struct RemoveReference {
        typedef T type;
    };
    template <class T>
    struct RemoveReference<T&> {
        typedef T type;
    };
    template <class T>
    struct RemoveReference<T&&> {
        typedef T type;
    };

    template <class T, class = void>
    struct AddReference {
        using LValue = T;
        using RValue = T;
    };

    template <class T>
    struct AddReference<T, VoidT<T&>> {
        using LValue = T&;
        using RValue = T&&;
    };

    template <class T>
    struct AddLvalueReference {
        using type = typename AddReference<T>::LValue;
    };
    template <class T>
    struct AddRvalueReference {
        using type = typename AddReference<T>::RValue;
    };

    template <class T>
    struct AddLValueRefIfNotPtr {
        using type = typename AddReference<T>::LValue;
    };

    template <typename T>
    struct AddLValueRefIfNotPtr<T*> {
        using type = T*;
    };

    template <typename T>
    using AddLValueRefIfNotPtrT = typename AddLValueRefIfNotPtr<T>::type;

    // declval
    template <class T>
    typename AddRvalueReference<T>::type declval() noexcept;

    // details
    namespace detail {
        template <class T>
        struct TypeIdentity {
            using type = T;
        };

        template <class T>
        auto TryAddLvalueReference(int) -> TypeIdentity<T&>;
        template <class T>
        auto TryAddLvalueReference(...) -> TypeIdentity<T>;

        template <class T>
        auto TryAddRvalueReference(int) -> TypeIdentity<T&&>;
        template <class T>
        auto TryAddRvalueReference(...) -> TypeIdentity<T>;

        template <typename B>
        TrueType TestPrePtrConvertible(const volatile B*);
        template <typename>
        FalseType TestPrePtrConvertible(const volatile void*);

        template <typename, typename>
        auto TestPreIsBaseOf(...) -> TrueType;

        template <typename B, typename D>
        auto TestPreIsBaseOf(int) -> decltype(TestPrePtrConvertible<B>(static_cast<D*>(nullptr)));

        template <class T>
        IntegralConstant<bool, !IsUnion<T>::value> Test(int T::*);

        template <class>
        FalseType Test(...);

        template <class T>
        auto TestReturnable(int) -> decltype(void(static_cast<T (*)()>(nullptr)), TrueType{});
        template <class>
        auto TestReturnable(...) -> FalseType;

        template <class From, class To>
        auto TestImplicitlyConvertible(int) -> decltype(void(declval<void (&)(To)>()(declval<From>())), TrueType{});
        template <class, class>
        auto TestImplicitlyConvertible(...) -> FalseType;

        template <class T>
        struct ReferenceWrapper : FalseType {};
        template <class U>
        struct ReferenceWrapper<ReferenceWrapper<U>> : TrueType {};

    } // namespace detail

    // remove/add qualifiers

    template <class T>
    struct RemoveCv {
        typedef T type;
        template <template <class> class Fn>
        using Apply = Fn<T>;
    };
    template <class T>
    struct RemoveCv<const T> {
        typedef T type;

        template <template <class> class Fn>
        using Apply = const Fn<T>;
    };
    template <class T>
    struct RemoveCv<volatile T> {
        typedef T type;

        template <template <class> class Fn>
        using Apply = volatile Fn<T>;
    };
    template <class T>
    struct RemoveCv<const volatile T> {
        typedef T type;

        template <template <class> class Fn>
        using Apply = const volatile Fn<T>;
    };

    template <class T>
    struct RemoveConst {
        typedef T type;
    };
    template <class T>
    struct RemoveConst<const T> {
        typedef T type;
    };

    template <class T>
    struct RemoveVolatile {
        typedef T type;
    };
    template <class T>
    struct RemoveVolatile<volatile T> {
        typedef T type;
    };

    template <class T>
    struct RemoveExtent {
        typedef T type;
    };

    template <class T>
    struct RemoveExtent<T[]> {
        typedef T type;
    };

    template <class T, size_t N>
    struct RemoveExtent<T[N]> {
        typedef T type;
    };

    template <class T>
    using RemoveReferenceT = typename RemoveReference<T>::type;

    template <class T>
    struct AddCv {
        typedef const volatile T type;
    };

    template <class T>
    using AddCvT = typename AddCv<T>::type;

    template <class T>
    struct AddConst {
        typedef const T type;
    };

    template <class T>
    struct AddConst<T&> {
        typedef const T& type;
    };

    template <class T>
    using AddConstT = typename AddConst<T>::type;

    template <class T>
    struct AddVolatile {
        typedef volatile T type;
    };

    template <class T>
    using AddVolatileT = typename AddVolatile<T>::type;

    template <class T, class = void>
    struct AddPointerImpl {
        using type = T;
    };

    template <class T>
    struct AddPointerImpl<T, VoidT<typename RemoveReference<T>::type*>> {
        using type = typename RemoveReference<T>::type*;
    };

    template <class T>
    struct AddPointer {
        using type = typename AddPointerImpl<T>::type;
    };

    // is same
    template <class T, class U>
    struct IsSame : FalseType {};

    template <class T>
    struct IsSame<T, T> : TrueType {};

    // isvoid
    template <class T>
    struct IsVoid : IsSame<void, typename RemoveCv<T>::type> {};

    // is lvalue reference
    template <class T>
    struct IsLvalueReference : FalseType {};
    template <class T>
    struct IsLvalueReference<T&> : TrueType {};

    // enable if
    template <bool B, class T = void>
    struct EnableIf {};

    template <class T>
    struct EnableIf<true, T> {
        typedef T type;
    };

    template <bool B, class T = void>
    using EnableIfT = typename EnableIf<B, T>::type;

    // conjunction
    template <bool FirstVal, class First, class... Rest>
    struct ConjunctionImpl { // handle false trait or last trait
        using type = First;
    };

    template <class True, class Next, class... Rest>
    struct ConjunctionImpl<true, True, Next, Rest...> { // the first trait is true, try the next one
        using type = typename ConjunctionImpl<Next::value, Next, Rest...>::type;
    };

    template <class... Traits>
    struct Conjunction : TrueType {};

    template <class First, class... Rest>
    struct Conjunction<First, Rest...> : ConjunctionImpl<First::value, First, Rest...>::type {};

    template <class... Traits>
    inline constexpr bool ConjunctionV = Conjunction<Traits...>::value;

    // disjunction
    template <bool FirstValue, class First, class... Rest>
    struct DisjunctionBase {
        using type = First;
    };

    template <class False, class Next, class... Rest>
    struct DisjunctionBase<false, False, Next, Rest...> { // first trait is false, try the next trait
        using type = typename DisjunctionBase<Next::value, Next, Rest...>::type;
    };

    template <class... Traits>
    struct Disjunction : FalseType {}; // If Traits is empty, false_type

    template <class First, class... Rest>
    struct Disjunction<First, Rest...> : DisjunctionBase<First::value, First, Rest...>::type {};

    template <class... Traits>
    inline constexpr bool DisjunctionV = Disjunction<Traits...>::value;

    template <class T, class V, class... Others>
    struct AllOfBase : FalseType {};

    template <class T, class... Others>
    struct AllOfBase<T, T, Others...> : AllOfBase<T, Others...> {};

    template <class T>
    struct AllOfBase<T, T> : TrueType {};

    template <class T, class... Others>
    struct AllOf : AllOfBase<T, Others...> {};

    template <class T>
    struct AllOf<T> : TrueType {};

    template <class T, class... Others>
    inline constexpr bool AllOfV = AllOf<T, Others...>::value;

    template <class T>
    [[nodiscard]] constexpr T* addressof(T& val) noexcept {
        return __builtin_addressof(val);
    }

    template <class T, class... Types>
    inline constexpr bool IsAnyOfV = DisjunctionV<IsSame<T, Types>...>;

    [[nodiscard]] consteval bool is_constant_evaluated() noexcept { return __builtin_is_constant_evaluated(); }

    template <class T>
    inline constexpr bool IsIntegralV =
    IsAnyOfV<typename RemoveCv<T>::type, bool, char, signed char, unsigned char, wchar_t, char16_t, char32_t, short,
             unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

    template <class T>
    inline constexpr bool IsSizeV =
    IsAnyOfV<typename RemoveCv<T>::type, unsigned int, unsigned long, unsigned long long>;

    template <class T>
    struct IsIntegral : BoolConstant<IsIntegralV<T>> {};

    template <class T>
    struct IsSize : BoolConstant<IsSizeV<T>> {};

    template <class T>
    inline constexpr bool IsNonboolIntegral = IsIntegralV<T> && !IsSame<typename RemoveCv<T>::type, bool>::value;

    // ptrsize
    using PtrSize = ConditionalT<sizeof(void*) == 8, uint64_t, uint32_t>;

    class Undefined;

    // Given Template<T, ...> return T, otherwise invalid.
    template <typename T>
    struct GetFirstArg {
        using type = Undefined;
    };

    template <template <typename, typename...> class Template, typename T, typename... Types>
    struct GetFirstArg<Template<T, Types...>> {
        using type = T;
    };

    template <typename T>
    using GetFirstArgT = typename GetFirstArg<T>::type;

    // Given Template<T, ...> and U return Template<U, ...>, otherwise invalid.
    template <typename T, typename U>
    struct ReplaceFirstArg {};

    template <template <typename, typename...> class Template, typename U, typename T, typename... Types>
    struct ReplaceFirstArg<Template<T, Types...>, U> {
        using type = Template<U, Types...>;
    };

    template <typename T, typename U>
    using ReplaceFirstArgT = typename ReplaceFirstArg<T, U>::type;

    template <typename T>
    using MakeNotVoid = typename Conditional<IsVoid<T>::value, Undefined, T>::type;

    template <typename Default, typename AlwaysVoid, template <typename...> class Op, typename... Args>
    struct Detector {
        using value_t = FalseType;
        using type = Default;
    };

    template <typename Default, template <typename...> class Op, typename... Args>
    struct Detector<Default, VoidT<Op<Args...>>, Op, Args...> {
        using value_t = TrueType;
        using type = Op<Args...>;
    };

    template <typename Default, template <typename...> class Op, typename... Args>
    using DetectedOr = Detector<Default, void, Op, Args...>;

    template <typename Default, template <typename...> class Op, typename... Args>
    using DetectedOrT = typename DetectedOr<Default, Op, Args...>::type;
} // namespace ARLib
