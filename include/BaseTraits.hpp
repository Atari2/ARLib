#pragma once
#include "Types.hpp"
/*
    the very basic type traits and some of the more complex ones (such as std::invoke_result)
    are derived from MSVC's STL implementation https://github.com/microsoft/STL
	which operates under the Apache License with LLVM exception, with notice included here:

    Copyright (c) Microsoft Corporation.
    SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

namespace ARLib {
// integral constant
template <class T, T Val>
struct IntegralConstant {
    constexpr static T value = Val;

    using value_type = T;
    using type       = IntegralConstant;
    constexpr operator value_type() const noexcept { return value; }
    [[nodiscard]] constexpr value_type operator()() const noexcept { return value; }
};
template <bool V>
using BoolConstant = IntegralConstant<bool, V>;

using TrueType  = BoolConstant<true>;
using FalseType = BoolConstant<false>;
// aliases to std:: members that I have no clue how to implement
template <class T, class... Args>
struct ConstructibleImpl : BoolConstant<__is_constructible(T, Args...)> {};
template <class T, class... Args>
struct TriviallyConstructibleImpl : BoolConstant<__is_trivially_constructible(T, Args...)> {};
template <class T, class... Args>
struct NothrowConstructibleImpl : BoolConstant<__is_nothrow_constructible(T, Args...)> {};
template <class T, class U>
struct AssignableImpl : BoolConstant<__is_assignable(T, U)> {};
template <class T, class U>
struct TriviallyAssignableImpl : BoolConstant<__is_trivially_assignable(T, U)> {};
template <class T, class U>
struct NothrowAssignableImpl : BoolConstant<__is_nothrow_assignable(T, U)> {};
template <typename T>
struct Identity {
    using type = T;
};
template <class T>
struct IsUnion : BoolConstant<__is_union(T)> {};
template <class T>
struct IsEnum : BoolConstant<__is_enum(T)> {};
template <typename T>
constexpr inline bool IsEmptyV = __is_empty(T);
template <typename T>
constexpr inline bool IsFinalV = __is_final(T);
template <class T>
constexpr inline bool IsEnumV = IsEnum<T>::value;

template <typename...>
using VoidT = void;
// type traits

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
}    // namespace detail
template <class T>
struct IsTriviallyCopiable : BoolConstant<__is_trivially_copyable(T)> {};

template <class T>
constexpr inline bool IsTriviallyCopiableV = __is_trivially_copyable(T);
template <class T>
struct IsTrivial : BoolConstant<__is_trivially_constructible(T) && __is_trivially_copyable(T)> {};

template <class T>
constexpr inline bool IsTrivialV = __is_trivially_constructible(T) && __is_trivially_copyable(T);
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
constexpr inline bool IsReferenceV = IsReference<T>::value;
template <class T>
struct IsArray : FalseType {};
template <class T>
struct IsArray<T[]> : TrueType {};
template <class T, size_t N>
struct IsArray<T[N]> : TrueType {};
#ifdef COMPILER_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4180)
#endif
template <class T>
constexpr inline bool IsArrayV = IsArray<T>::value;
template <class T>
struct IsFunction : IntegralConstant<bool, !IsConst<const T>::value && !IsReference<T>::value> {};

template <class T>
constexpr inline bool IsFunctionV = IsFunction<T>::value;

#ifdef COMPILER_MSVC
    #pragma warning(pop)
#endif
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

template <class T>
using RemoveReferenceT = typename RemoveReference<T>::type;
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
}    // namespace detail
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

template <typename T>
struct RemoveAllExtents {
    typedef T type;
};
template <typename T, size_t Size>
struct RemoveAllExtents<T[Size]> {
    typedef typename RemoveAllExtents<T>::type type;
};
template <typename T>
struct RemoveAllExtents<T[]> {
    typedef typename RemoveAllExtents<T>::type type;
};

template <typename T>
using RemoveAllExtentsT = typename RemoveAllExtents<T>::type;

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
    using type = const T;
};
template <class T>
struct AddConst<T&> {
    using type = const T&;
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

template <class T>
using AddPointerT = typename AddPointer<T>::type;
// is same
template <class T, class U>
struct IsSame : FalseType {};
template <class T>
struct IsSame<T, T> : TrueType {};
template <class T, class U>
struct IsSameCvRef :
    IsSame<typename RemoveCv<RemoveReferenceT<T>>::type, typename RemoveCv<RemoveReferenceT<U>>::type> {};
// isvoid
template <class T>
struct IsVoid : IsSame<void, typename RemoveCv<T>::type> {};
template <class T>
constexpr inline bool IsVoidV = IsVoid<T>::value;
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
struct ConjunctionImpl {    // handle false trait or last trait
    using type = First;
};
template <class True, class Next, class... Rest>
struct ConjunctionImpl<true, True, Next, Rest...> {    // the first trait is true, try the next one
    using type = typename ConjunctionImpl<Next::value, Next, Rest...>::type;
};
template <class... Traits>
struct Conjunction : TrueType {};
template <class First, class... Rest>
struct Conjunction<First, Rest...> : ConjunctionImpl<First::value, First, Rest...>::type {};

template <class... Traits>
constexpr inline bool ConjunctionV = Conjunction<Traits...>::value;
// disjunction
template <bool FirstValue, class First, class... Rest>
struct DisjunctionBase {
    using type = First;
};
template <class False, class Next, class... Rest>
struct DisjunctionBase<false, False, Next, Rest...> {    // first trait is false, try the next trait
    using type = typename DisjunctionBase<Next::value, Next, Rest...>::type;
};
template <class... Traits>
struct Disjunction : FalseType {};    // If Traits is empty, false_type
template <class First, class... Rest>
struct Disjunction<First, Rest...> : DisjunctionBase<First::value, First, Rest...>::type {};

template <class... Traits>
constexpr inline bool DisjunctionV = Disjunction<Traits...>::value;
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
constexpr inline bool AllOfV = AllOf<T, Others...>::value;
template <class T>
[[nodiscard]] constexpr T* addressof(T& val) noexcept {
    return __builtin_addressof(val);
}

template <class T, class... Types>
constexpr inline bool IsAnyOfV = DisjunctionV<IsSame<T, Types>...>;

template <class T, class... Types>
constexpr inline bool IsAnyOfCvRefV = DisjunctionV<IsSameCvRef<T, Types>...>;
[[nodiscard]] constexpr bool is_constant_evaluated() noexcept {
    return __builtin_is_constant_evaluated();
}

template <class T>
constexpr inline bool IsIntegralV = IsAnyOfV<
typename RemoveCv<T>::type, bool, char, signed char, unsigned char, wchar_t, char16_t, char32_t, short, unsigned short,
int, unsigned int, long, unsigned long, long long, unsigned long long>;

template <class T>
constexpr inline bool IsFloatingPointV = IsAnyOfV<typename RemoveCv<T>::type, float, double, long double>;

template <class T>
constexpr inline bool IsNumericV = IsIntegralV<T> || IsFloatingPointV<T>;

template <class T>
constexpr inline bool IsSizeV = IsAnyOfV<typename RemoveCv<T>::type, unsigned int, unsigned long, unsigned long long>;
template <class T>
struct IsIntegral : BoolConstant<IsIntegralV<T>> {};
template <class T>
struct IsSize : BoolConstant<IsSizeV<T>> {};

template <class T>
constexpr inline bool IsNonboolIntegral = IsIntegralV<T> && !IsSame<typename RemoveCv<T>::type, bool>::value;

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
    using type    = Default;
};
template <typename Default, template <typename...> class Op, typename... Args>
struct Detector<Default, VoidT<Op<Args...>>, Op, Args...> {
    using value_t = TrueType;
    using type    = Op<Args...>;
};

template <typename Default, template <typename...> class Op, typename... Args>
using DetectedOr = Detector<Default, void, Op, Args...>;

template <typename Default, template <typename...> class Op, typename... Args>
using DetectedOrT = typename DetectedOr<Default, Op, Args...>::type;
template <typename, unsigned Uint>
struct Extent : public IntegralConstant<size_t, 0> {};
template <typename T, unsigned Uint, size_t Size>
struct Extent<T[Size], Uint> : public IntegralConstant<size_t, Uint == 0 ? Size : Extent<T, Uint - 1>::value> {};
template <typename T, unsigned Uint>
struct Extent<T[], Uint> : public IntegralConstant<size_t, Uint == 0 ? 0 : Extent<T, Uint - 1>::value> {};
template <typename T, unsigned Idx = 0>
constexpr size_t ExtentV = Extent<T, Idx>::value;
template <typename T>
struct IsNullPointer : FalseType {};
template <>
struct IsNullPointer<decltype(nullptr)> : TrueType {};
template <typename T>
constexpr inline bool IsNullPointerV = IsNullPointer<T>::value;

template <typename T>
constexpr inline bool IsArithmeticV = IsIntegralV<T> || IsFloatingPointV<T>;

template <class>
constexpr inline bool IsPointerV = false;

template <class T>
constexpr inline bool IsPointerV<T*> = true;

template <class T>
constexpr inline bool IsPointerV<T* const> = true;

template <class T>
constexpr inline bool IsPointerV<T* volatile> = true;

template <class T>
constexpr inline bool IsPointerV<T* const volatile> = true;
template <class T>
struct IsPointer : BoolConstant<IsPointerV<T>> {};

template <typename>
struct IsMemberFunctionPointerHelper : public FalseType {};
template <typename T, typename C>
struct IsMemberFunctionPointerHelper<T C::*> : public IsFunction<T>::type {};
template <typename T>
struct IsMemberFunctionPointer : public IsMemberFunctionPointerHelper<typename RemoveCv<T>::type>::type {};
template <typename T>
constexpr bool IsMemberFunctionPointerV = IsMemberFunctionPointer<T>::value;

template <typename T>
constexpr inline bool IsScalarV =
IsArithmeticV<T> || IsEnumV<T> || IsPointerV<T> || IsMemberFunctionPointerV<T> || IsNullPointerV<T>;

#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
struct IsDestructibleSafeImplBase {
    template <typename T, typename = decltype(declval<T&>().~T())>
    static TrueType test(int);

    template <typename>
    static FalseType test(...);
};
template <typename T>
struct IsDestructibleSafeImpl : public IsDestructibleSafeImplBase {
    typedef decltype(test<T>(0)) type;
};
template <typename T>
constexpr inline bool IsArrayUknownBounds = IsArrayV<T> && !ExtentV<T>;
template <
typename T, bool = IsVoidV<T> || IsArrayUknownBounds<T> || IsFunctionV<T>, bool = IsReferenceV<T> || IsScalarV<T>>
struct IsDestructibleSafe;
template <typename T>
struct IsDestructibleSafe<T, false, false> : public IsDestructibleSafeImpl<RemoveAllExtentsT<T>>::type {};
template <typename T>
struct IsDestructibleSafe<T, true, false> : public FalseType {};
template <typename T>
struct IsDestructibleSafe<T, false, true> : public TrueType {};
#endif

#if defined(COMPILER_MSVC)
template <class T>
struct IsTriviallyDestructible : BoolConstant<__is_trivially_destructible(T)> {};

template <class T>
constexpr inline bool IsTriviallyDestructibleV = __is_trivially_destructible(T);

#else
template <class T>
struct IsTriviallyDestructible : detail::And<IsDestructibleSafe<T>, BoolConstant<__has_trivial_destructor(T)>> {};

template <class T>
constexpr inline bool IsTriviallyDestructibleV = IsTriviallyDestructible<T>::value;

#endif

}    // namespace ARLib
