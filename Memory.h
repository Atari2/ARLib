#pragma once
#include "Concepts.h"
#include "SourceLocation.h"
#include "TypeTraits.h"
#include "cstring_compat.h"

#ifdef DEBUG_NEW_DELETE
#define LOC loc
#else
#define LOC
#endif

namespace ARLib {
    template <class T>
    T* alloc(SourceLocation LOC = SourceLocation::current()) {
#ifdef DEBUG_NEW_DELETE
        printf("Allocated from `%s` in %s [%u:%u]\n", loc.function_name(), loc.file_name(), loc.line(), loc.column());
#endif
        return new T;
    }

    template <class T, typename... Args>
    T* construct(Args... args, SourceLocation LOC = SourceLocation::current()) requires Constructible<T, Args...> {
#ifdef DEBUG_NEW_DELETE
        printf("Constructed from `%s` in %s [%u:%u]\n", loc.function_name(), loc.file_name(), loc.line(), loc.column());
#endif
        return new T{args...};
    }

    template <class T>
    T* alloc(size_t count, SourceLocation LOC = SourceLocation::current()) {
#ifdef DEBUG_NEW_DELETE
        printf("Allocated from `%s` in %s [%u:%u]\n", loc.function_name(), loc.file_name(), loc.line(), loc.column());
#endif
        return new T[count];
    }

    template <class To, class From,
              EnableIfT<ConjunctionV<BoolConstant<sizeof(To) == sizeof(From)>, IsTriviallyCopiable<To>,
                                     IsTriviallyCopiable<From>>,
                        int> = 0>
    [[nodiscard]] constexpr To BitCast(const From& val) noexcept {
        return __builtin_bit_cast(To, val);
    }

    // dst and src may not overlap
    template <CopyAssignable T>
    constexpr void ConditionalBitCopy(T* dst, const T* src, size_t count) {
        if (!dst || !src || count == 0) return;
        if constexpr (IsTriviallyCopiableV<T>) {
            memcpy(dst, src, count * sizeof(T));
        } else {
            for (size_t i = 0; i < count; i++) {
                dst[i] = src[i];
            }
        }
    }

    // dst and src may not overlap
    template <MoveAssignable T>
    constexpr void ConditionalBitMove(T* dst, T*& src, size_t count) {
        if (!dst || !src || count == 0) return;
        if constexpr (IsTriviallyCopiableV<T>) {
            memcpy(dst, src, count * sizeof(T));
        } else {
            for (size_t i = 0; i < count; i++) {
                dst[i] = move(src[i]);
            }
        }
        delete[] src;
        src = nullptr;
    }
} // namespace ARLib
