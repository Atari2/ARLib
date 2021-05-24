#pragma once
#include <type_traits>
#include "Types.h"

namespace ARLib {
    // the overwhelming majority of the type traits are copy-pasted from cppreference
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

    template <class T>
    using IsUnion = std::is_union<T>;

    // type traits

    // integral constant
    template <class T, T Val>
    struct IntegralConstant {
        static constexpr T value = Val;

        using value_type = T;
        using type = IntegralConstant;

        constexpr operator value_type() const noexcept {
            return value;
        }

        [[nodiscard]] constexpr value_type operator()() const noexcept {
            return value;
        }
    };


    template <bool V>
    using BoolConstant = IntegralConstant<bool, V>;

    using TrueType = BoolConstant<true>;
    using FalseType = BoolConstant<false>;

    // details
    namespace detail {
        template <class T>
        struct TypeIdentity { using type = T; };

        template <class T>
        auto TryAddLvalueReference(int)->TypeIdentity<T&>;
        template <class T>
        auto TryAddLvalueReference(...)->TypeIdentity<T>;

        template <class T>
        auto TryAddRvalueReference(int)->TypeIdentity<T&&>;
        template <class T>
        auto TryAddRvalueReference(...)->TypeIdentity<T>;

        template <typename B>
        TrueType  TestPrePtrConvertible(const volatile B*);
        template <typename>
        FalseType TestPrePtrConvertible(const volatile void*);

        template <typename, typename>
        auto TestPreIsBaseOf(...)->TrueType;

        template <typename B, typename D>
        auto TestPreIsBaseOf(int) -> decltype(TestPrePtrConvertible<B>(static_cast<D*>(nullptr)));

        template <class T>
        IntegralConstant<bool, !IsUnion<T>::value> Test(int T::*);

        template <class>
        FalseType Test(...);

        template<class T>
        auto TestReturnable(int) -> decltype(
            void(static_cast<T(*)()>(nullptr)), TrueType{}
        );
        template<class>
        auto TestReturnable(...)->FalseType;

        template<class From, class To>
        auto TestImplicitlyConvertible(int) -> decltype(
            void(std::declval<void(&)(To)>()(std::declval<From>())), TrueType{}
        );
        template<class, class>
        auto TestImplicitlyConvertible(...)->FalseType;
    }

    // remove/add qualifiers
    template< class T > struct RemoveReference { typedef T type; };
    template< class T > struct RemoveReference<T&> { typedef T type; };
    template< class T > struct RemoveReference<T&&> { typedef T type; };

    template< class T > struct RemoveCv { typedef T type; };
    template< class T > struct RemoveCv<const T> { typedef T type; };
    template< class T > struct RemoveCv<volatile T> { typedef T type; };
    template< class T > struct RemoveCv<const volatile T> { typedef T type; };

    template< class T > struct RemoveConst { typedef T type; };
    template< class T > struct RemoveConst<const T> { typedef T type; };

    template< class T > struct RemoveVolatile { typedef T type; };
    template< class T > struct RemoveVolatile<volatile T> { typedef T type; };

    template <class T>
    using RemoveReferenceT = typename RemoveReference<T>::type;

    template<class T> struct AddCv { typedef const volatile T type; };
    template<class T> struct AddConst { typedef const T type; };
    template<class T> struct AddVolatile { typedef volatile T type; };

    template <class T>
    struct AddLvalueReference : decltype(detail::TryAddLvalueReference<T>(0)) {};
    template <class T>
    struct AddRvalueReference : decltype(detail::TryAddRvalueReference<T>(0)) {};

    // is same
    template<class T, class U>
    struct IsSame : FalseType {};

    template<class T>
    struct IsSame<T, T> : TrueType {};

    // is lvalue reference
    template<class T> struct IsLvalueReference : FalseType {};
    template<class T> struct IsLvalueReference<T&> : TrueType {};

    // enable if
    template<bool B, class T = void>
    struct EnableIf {};

    template<class T>
    struct EnableIf<true, T> { typedef T type; };

    template <bool B, class T = void>
    using EnableIfT = typename EnableIf<B, T>::type;

    // conditional
    template<bool cond, class TrueT, class FalseT>
    struct Conditional {
        using type = TrueT;
    };

    template<class TrueT, class FalseT>
    struct Conditional<false, TrueT, FalseT> {
        using type = FalseT;
    };

    template<bool cond, class TrueType, class FalseType>
    using ConditionalT = typename Conditional<cond, TrueType, FalseType>::type;

    // conjuction
    template<class...> struct Conjunction : TrueType { };
    template<class B1> struct Conjunction<B1> : B1 { };
    template<class B1, class... Bn>
    struct Conjunction<B1, Bn...>
        : Conditional<bool(B1::value), Conjunction<Bn...>, B1> {};

    // declval
    template<class T>
    typename AddRvalueReference<T>::type declval() noexcept;

    // ptrsize
    using PtrSize = ConditionalT<sizeof(void*) == 8, uint64_t, uint32_t>;
}