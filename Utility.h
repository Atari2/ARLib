#pragma once
#include "Compat.h"
#include "Concepts.h"
#include "Macros.h"
#include "NumberTraits.h"

namespace ARLib {

    template <typename T, T... Idx>
    struct IntegerSequence {
        static_assert(IsIntegralV<T>, "Integer sequence requires T to be an integral type");
        using value_type = T;
        static constexpr size_t size() noexcept { return sizeof...(Idx); }
    };

    template <typename T, T Num>
    using MakeIntegerSequence
#if defined(WINDOWS)
    = __make_integer_seq<IntegerSequence, T, Num>;
#else
#if __has_builtin(__make_integer_seq)
    = __make_integer_seq<IntegerSequence, T, Num>;
#else
    = IntegerSequence<T, __integer_pack(Num)...>;
#endif
#endif

    template <size_t... Vals>
    using IndexSequence = IntegerSequence<size_t, Vals...>;

    template <size_t Size>
    using MakeIndexSequence = MakeIntegerSequence<size_t, Size>;

    template <class... Types>
    using IndexSequenceFor = MakeIndexSequence<sizeof...(Types)>;

    template <class T>
    inline constexpr T&& Forward(typename RemoveReference<T>::type& t) noexcept {
        return static_cast<T&&>(t);
    }

    template <class T>
    inline constexpr T&& Forward(typename RemoveReference<T>::type&& t) noexcept {
        return static_cast<T&&>(t);
    }

    template <typename T>
    constexpr RemoveReferenceT<T>&& move(T&& t) noexcept {
        return static_cast<RemoveReferenceT<T>&&>(t);
    }

    template <typename T>
    constexpr inline void swap(T& a, T& b) noexcept requires NothrowMoveAssignable<T> && NothrowMoveConstructible<T> {
        T tmp = move(a);
        a = move(b);
        b = move(tmp);
    }

    template <typename T, size_t N>
    constexpr inline void swap(T (&a)[N], T (&b)[N]) noexcept requires Swappable<T> {
        for (size_t n = 0; n < N; ++n)
            swap(a[n], b[n]);
    }

    template <typename C, size_t N>
    consteval size_t sizeof_array(C (&)[N]) {
        return N;
    }

    template <typename T>
    constexpr size_t ptrdiff(const T* begin, const T* end) {
        return static_cast<size_t>(end - begin);
    }

    consteval auto operator""_u8(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint8_t>::max, "Can't convert to uint8_t");
        return static_cast<uint8_t>(num);
    }

    consteval auto operator""_i8(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint8_t>::max, "Can't convert to int8_t");
        return static_cast<int8_t>(num);
    }

    consteval auto operator""_u16(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint16_t>::max, "Can't convert to uint16_t");
        return static_cast<uint16_t>(num);
    }

    consteval auto operator""_i16(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint16_t>::max, "Can't convert to uint16_t");
        return static_cast<int16_t>(num);
    }

    consteval auto operator""_u32(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint32_t>::max, "Can't convert to uint32_t");
        return static_cast<uint32_t>(num);
    }

    consteval auto operator""_i32(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint32_t>::max, "Can't convert to uint32_t");
        return static_cast<int32_t>(num);
    }

    consteval auto operator""_u64(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint64_t>::max, "Can't convert to uint64_t");
        return static_cast<uint64_t>(num);
    }

    consteval auto operator""_i64(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint64_t>::max, "Can't convert to uint64_t");
        return static_cast<int64_t>(num);
    }

    consteval auto operator""_sz(const unsigned long long num) {
        CONSTEVAL_STATIC_ASSERT(num <= NumberTraits<uint64_t>::max, "Can't convert to size_t");
        return static_cast<size_t>(num);
    }

} // namespace ARLib
