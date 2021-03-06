#pragma once
#include "Concepts.h"
#include "TypeTraits.h"
#include "cstring_compat.h"
#include "Allocator.h"

namespace ARLib {
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
        deallocate<T, DeallocType::Multiple>(src);
        src = nullptr;
    }
} // namespace ARLib
