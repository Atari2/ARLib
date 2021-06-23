#pragma once
#include "TypeTraits.h"

namespace ARLib {
    template <class To, class From,
              EnableIfT<ConjunctionV<BoolConstant<sizeof(To) == sizeof(From)>, IsTriviallyCopiable<To>,
                                     IsTriviallyCopiable<From>>,
                        int> = 0>
    [[nodiscard]] constexpr To BitCast(const From& val) noexcept {
        return __builtin_bit_cast(To, val);
    }
} // namespace ARLib

using ARLib::BitCast;