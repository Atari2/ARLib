#pragma once
#include "BaseTraits.h"
#include "Utility.h"

namespace ARLib {
    
    template<typename Ptr>
    struct PointerTraits
    {
    private:
        template<typename T>
        using element_type_internal = typename T::element_type;

        template<typename T>
        using difference_type_internal = typename T::difference_type;

        template<typename T, typename U, typename = void>
        struct rebind_internal : ReplaceFirstArg<T, U> { };

        template<typename T, typename U>
        struct rebind_internal<T, U, VoidT<typename T::template rebind<U>>>
        {
            using type = typename T::template rebind<U>;
        };

    public:
        /// The pointer type.
        using pointer = Ptr;

        /// The type pointed to.
        using element_type
            = DetectedOrT<GetFirstArgT<Ptr>, element_type_internal, Ptr>;

        /// The type used to represent the difference between two pointers.
        using difference_type
            = DetectedOrT<ptrdiff_t, difference_type_internal, Ptr>;

        /// A pointer to a different type.
        template<typename U>
        using rebind = typename rebind_internal<Ptr, U>::type;

        static Ptr
            pointer_to(MakeNotVoid<element_type>& e)
        {
            return Ptr::pointer_to(e);
        }

        static_assert(!IsSame<element_type, Undefined>::value,
            "pointer type defines element_type or is like SomePointer<T, Args>");
    };


    template<typename T>
    struct PointerTraits<T*>
    {
        /// The pointer type
        typedef T* pointer;
        /// The type pointed to
        typedef T  element_type;
        /// Type used to represent the difference between two pointers
        typedef ptrdiff_t difference_type;

        template<typename U>
        using rebind = U*;

        static constexpr pointer
            pointer_to(MakeNotVoid<element_type>& r) noexcept
        {
            return addressof(r);
        }
    };

    // isclass
    template <class T>
    struct IsClass : decltype(detail::Test<T>(nullptr)) {};

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


    template <bool>
    struct Select { 
        template <class T1, class>
        using Apply = T1;
    };

    template <>
    struct Select<false> {
        template <class, class T2>
        using Apply = T2;
    };


    template <size_t>
    struct MakeUnsignedHelper;

    template <>
    struct MakeUnsignedHelper<1> {
        template <class>
        using Apply = uint8_t;
    };

    template <>
    struct MakeUnsignedHelper<2> {
        template <class>
        using Apply = uint16_t;
    };

    template <>
    struct MakeUnsignedHelper<4> {
        template <class>
        using Apply = uint32_t;
    };

    template <>
    struct MakeUnsignedHelper<8> {
        template <class>
        using Apply = uint64_t;
    };

    template <class T>
    using MakeUnsignedBase = typename MakeUnsignedHelper<sizeof(T)>::template Apply<T>;

    template <class T>
    struct MakeUnsigned {
        static_assert(IsNonboolIntegral<T> || IsEnumV<T>, "make_unsigned<T> requires a non bool integral type.");

        using type = typename RemoveCv<T>::template Apply<MakeUnsignedBase>;
    };

    template <class T>
    using MakeUnsignedT = typename MakeUnsigned<T>::type;

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

    namespace detail {

        template<class T>
        struct InvokeImpl {
            template<class F, class... Args>
            static auto call(F&& f, Args&&... args)
                -> decltype(Forward<F>(f)(Forward<Args>(args)...));
        };

        template<class B, class MT>
        struct InvokeImpl<MT B::*> {
            template<class T, class Td = typename Decay<T>::type,
                class = typename EnableIf<BaseOf<B, Td>::value>::type
            >
                static auto get(T&& t)->T&&;

            template<class T, class Td = typename Decay<T>::type,
                class = typename EnableIf<ReferenceWrapper<Td>::value>::type
            >
                static auto get(T&& t) -> decltype(t.get());

            template<class T, class Td = typename Decay<T>::type,
                class = typename EnableIf<!BaseOf<B, Td>::value>::type,
                class = typename EnableIf<!ReferenceWrapper<Td>::value>::type
            >
                static auto get(T&& t) -> decltype(*Forward<T>(t));

            template<class T, class... Args, class MT1,
                class = typename EnableIf<IsFunction<MT1>::value>::type
            >
                static auto call(MT1 B::* pmf, T&& t, Args&&... args)
                -> decltype((InvokeImpl::get(Forward<T>(t)).*pmf)(Forward<Args>(args)...));

            template<class T>
            static auto call(MT B::* pmd, T&& t)
                -> decltype(InvokeImpl::get(Forward<T>(t)).*pmd);
        };

        template<class F, class... Args, class Fd = typename Decay<F>::type>
        auto INVOKE(F&& f, Args&&... args)
            -> decltype(InvokeImpl<Fd>::call(Forward<F>(f), Forward<Args>(args)...));

        template <typename AlwaysVoid, typename, typename...>
        struct InvokeResult { };
        template <typename F, typename...Args>
        struct InvokeResult<decltype(void(detail::INVOKE(declval<F>(), declval<Args>()...))),
            F, Args...> {
            using type = decltype(detail::INVOKE(declval<F>(), declval<Args>()...));
        };
    }

    template <class T, bool = IsEnumV<T>>
    struct UnderlyingTypeImpl {
        using type = __underlying_type(T);
    };

    template <class T>
    struct UnderlyingTypeImpl<T, false> {};

    template <class T>
    struct UnderlyingType : UnderlyingTypeImpl<T> {}; // determine underlying type for enum

    template <class T>
    using UnderlyingTypeT = typename UnderlyingType<T>::type;

    template <class> struct ResultOf;

    template <class F, class... ArgTypes>
    struct ResultOf<F(ArgTypes...)> : detail::InvokeResult<void, F, ArgTypes...> {};

    template <class F, class... ArgTypes>
    struct InvokeResult : detail::InvokeResult<void, F, ArgTypes...> {};

    template< class T >
    using ResultOfT = typename ResultOf<T>::type;

    template< class F, class... ArgTypes>
    using InvokeResultT = typename InvokeResult<F, ArgTypes...>::type;

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

    template<class T>
    inline constexpr bool IsConstV = IsConst<T>::value;

    template <class>
    inline constexpr bool IsVolatileV = false;

    template <class T>
    inline constexpr bool IsVolatileV<volatile T> = true;

    template <class T>
    struct IsVolatile : BoolConstant<IsVolatileV<T>> {};

    template <class>
    inline constexpr bool IsPointerV = false;

    template <class T>
    inline constexpr bool IsPointerV<T*> = true;

    template <class T>
    inline constexpr bool IsPointerV<T* const> = true;

    template <class T>
    inline constexpr bool IsPointerV<T* volatile> = true;

    template <class T>
    inline constexpr bool IsPointerV<T* const volatile> = true;

    template <class T>
    struct IsPointer : BoolConstant<IsPointerV<T>> {};

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

    // Helper to SFINAE away emplace variadic template parameters constructors
    template <class T>
    struct EmplaceT {};

}
