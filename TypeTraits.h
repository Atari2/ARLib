#pragma once
#include "BaseTraits.h"
#include "Utility.h"

namespace ARLib {

    // isclass
    template <class T>
    struct IsClass : decltype(detail::Test<T>(nullptr)) {};

    // isvoid
    template< class T >
    struct IsVoid : IsSame<void, typename RemoveCv<T>::type> {};

    // convertible
    template<class From, class To>
    struct Convertible : IntegralConstant<bool,
        (decltype(detail::TestReturnable<To>(0))::value&&
            decltype(detail::TestImplicitlyConvertible<From, To>(0))::value) ||
        (IsVoid<From>::value && IsVoid<To>::value)
    > {};

    template<class From, class To>
    struct NothrowConvertible : Conjunction<IsVoid<From>, IsVoid<To>> {};


    // baseof
    template <typename Base, typename Derived>
    struct BaseOf :
        IntegralConstant <
        bool,
        IsClass<Base>::value && IsClass<Derived>::value &&
        decltype(detail::TestPreIsBaseOf<Base, Derived>(0))::value
        > {};

    // constructible in various ways -> todo in concepts
    template<class T>
    struct CopyConstructibleImpl :
        ConstructibleImpl<T, typename AddLvalueReference<
        typename AddConst<T>::type>::type> {};

    template<class T>
    struct TriviallyCopyConstructibleImpl :
        TriviallyConstructibleImpl<T, typename AddLvalueReference<
        typename AddConst<T>::type>::type> {};

    template<class T>
    struct NothrowCopyConstructibleImpl :
        NothrowConstructibleImpl<T, typename AddLvalueReference<
        typename AddConst<T>::type>::type> {};

    template<class T>
    struct MoveConstructibleImpl :
       ConstructibleImpl<T, typename AddRvalueReference<T>::type> {};

    template<class T>
    struct TriviallyMoveConstructibleImpl :
        TriviallyConstructibleImpl<T, typename AddRvalueReference<T>::type> {};

    template<class T>
    struct NothrowMoveConstructibleImpl :
        NothrowConstructibleImpl<T, typename AddRvalueReference<T>::type> {};


    // assignable in various ways -> todo in concepts
    template< class T>
    struct CopyAssignableImpl
        : AssignableImpl< typename AddLvalueReference<T>::type,
        typename AddLvalueReference<const T>::type> {};

    template< class T>
    struct TriviallyCopyAssignableImpl
        : TriviallyAssignableImpl< typename AddLvalueReference<T>::type,
        typename AddLvalueReference<const T>::type> {};

    template< class T>
    struct NothrowCopyAssignableImpl
        : NothrowAssignableImpl< typename AddLvalueReference<T>::type,
        typename AddLvalueReference<const T>::type> {};


    template< class T>
    struct MoveAssignableImpl
        : AssignableImpl< typename AddLvalueReference<T>::type,
        typename AddRvalueReference<T>::type> {};

    template< class T>
    struct TriviallyMoveAssignableImpl
        : TriviallyAssignableImpl< typename AddLvalueReference<T>::type,
        typename AddRvalueReference<T>::type> {};

    template< class T>
    struct NothrowMoveAssignableImpl
        : NothrowAssignableImpl< typename AddLvalueReference<T>::type,
        typename AddRvalueReference<T>::type> {};



    template< class T>
    struct DefaultConstructibleImpl : ConstructibleImpl<T> {};

    template< class T>
    struct TriviallyDefaultConstructibleImpl : TriviallyConstructibleImpl<T> {};

    template< class T>
    struct NothrowDefaultConstructibleImpl : NothrowConstructibleImpl<T> {};

    // helpers
    template< class T >
    using AddLvalueReferenceT = typename AddLvalueReference<T>::type;
    
    template< class T >
    using AddRvalueReferenceT = typename AddRvalueReference<T>::type;

    template< class T >
    struct Decay {
    private:
        typedef typename RemoveReference<T>::type U;
    public:
        typedef typename Conditional<
            IsArray<U>::value,
            typename RemoveExtent<U>::type*,
            typename Conditional<
            IsFunction<U>::value,
            typename AddPointer<U>::type,
            typename RemoveCv<U>::type
            >::type
        >::type type;
    };

    template <class T, class... Args>
    inline constexpr bool ConstructibleV = ConstructibleImpl<T, Args...>::type;
    template <class T, class... Args>
    inline constexpr bool TriviallyConstructibleV = TriviallyConstructibleImpl<T, Args...>::type;
    template <class T, class... Args>
    inline constexpr bool NothrowConstructibleV = NothrowConstructibleImpl<T, Args...>::type;


    template <class T, class U>
    inline constexpr bool AssignableV = AssignableImpl<T, U>::type;
    template <class T, class U>
    inline constexpr bool TriviallyAssignableV = TriviallyAssignableImpl<T, U>::type;
    template <class T, class U>
    inline constexpr bool NothrowAssignableV = NothrowAssignableImpl<T, U>::type;


    template <class T>
    inline constexpr bool MoveAssignableV = MoveAssignableImpl<T>::value;
    template <class T>
    inline constexpr bool CopyAssignableV = CopyAssignableImpl<T>::value;
    template <class T>
    inline constexpr bool MoveConstructibleV = MoveConstructibleImpl<T>::value;
    template <class T>
    inline constexpr bool CopyConstructibleV = CopyConstructibleImpl<T>::value;
    template <class T>
    inline constexpr bool NothrowMoveAssignableV = NothrowMoveAssignableImpl<T>::value;
    template <class T>
    inline constexpr bool NothrowCopyAssignableV = NothrowCopyAssignableImpl<T>::value;
    template <class T>
    inline constexpr bool NothrowMoveConstructibleV = NothrowMoveConstructibleImpl<T>::value;
    template <class T>
    inline constexpr bool NothrowCopyConstructibleV = NothrowCopyConstructibleImpl<T>::value;
    template <class T>
    inline constexpr bool TriviallyMoveAssignableV = TriviallyMoveAssignableImpl<T>::value;
    template <class T>
    inline constexpr bool TriviallyCopyAssignableV = TriviallyCopyAssignableImpl<T>::value;
    template <class T>
    inline constexpr bool TriviallyMoveConstructibleV = TriviallyMoveConstructibleImpl<T>::value;
    template <class T>
    inline constexpr bool TriviallyCopyConstructibleV = TriviallyCopyConstructibleImpl<T>::value;
    template< class T >
    using RemoveExtentT = typename RemoveExtent<T>::type;

    template< class T >
    inline constexpr bool DefaultConstructibleV = DefaultConstructibleImpl<T>::value;

    template< class T >
    inline constexpr bool TriviallyDefaultConstructibleV = TriviallyDefaultConstructibleImpl<T>::value;
    
    template< class T >
    inline constexpr bool NothrowDefaultConstructibleV = NothrowDefaultConstructibleImpl<T>::value;

    template< class Base, class Derived >
    inline constexpr bool BaseOfV = BaseOf<Base, Derived>::value;

    template<class T, class U>
    inline constexpr bool IsSameV = IsSame<T, U>::value;

    template< class From, class To >
    inline constexpr bool ConvertibleV = Convertible<From, To>::value;

    template< class From, class To >
    inline constexpr bool NothrowConvertibleV = NothrowConvertible<From, To>::value;

    template< class T >
    inline constexpr bool IsLvalueReferenceV = IsLvalueReference<T>::value;

    template <bool B>
    struct Supports{};

    template<>
    struct Supports<true> { static inline constexpr bool value = true; };

    template <bool B>
    inline constexpr bool SupportsV = Supports<B>::value;

}