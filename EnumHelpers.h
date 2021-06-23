#pragma once
#include "Concepts.h"
namespace ARLib {
    template <Enum T>
    auto ToUnderlying(T value) {
        return static_cast<UnderlyingTypeT<T>>(value);
    }

    template <Enum T>
    auto UpCast(auto value) {
        return static_cast<T>(value);
    }

#define BITFIELD_ENUM_OP_OR(E)                                                                                         \
    auto operator|(E self, E other) { return UpCast<E>(ToUnderlying(self) | ToUnderlying(other)); }
#define BITFIELD_ENUM_OP_AND(E)                                                                                        \
    auto operator&(E self, E other) { return UpCast<E>(ToUnderlying(self) & ToUnderlying(other)); }

#define BITFIELD_ENUM_OP_NONE(E)                                                                                       \
    auto operator!(E self) { return self == E::None; }
#define BITFIELD_ENUM_OP_LOG_AND(E)                                                                                    \
    auto operator&&(E self, E other) { return ToUnderlying(self) && ToUnderlying(other); }
#define BITFIELD_ENUM_OP_LOG_OR(E)                                                                                     \
    auto operator||(E self, E other) { return ToUnderlying(self) || ToUnderlying(other); }
#define BITFIELD_ENUM_OP_XOR(E)                                                                                        \
    auto operator^(E self, E other) { return UpCast<E>(ToUnderlying(self) ^ ToUnderlying(other)); }
#define BITFIELD_ENUM_OP_NOT(E)                                                                                        \
    auto operator~(E self) { return UpCast<E>(~ToUnderlying(self)); }

#define MAKE_BITFIELD_ENUM(E)                                                                                          \
    BITFIELD_ENUM_OP_OR(E)                                                                                             \
    BITFIELD_ENUM_OP_AND(E)                                                                                            \
    BITFIELD_ENUM_OP_NONE(E)                                                                                           \
    BITFIELD_ENUM_OP_LOG_AND(E)                                                                                        \
    BITFIELD_ENUM_OP_LOG_OR(E)                                                                                         \
    BITFIELD_ENUM_OP_XOR(E)                                                                                            \
    BITFIELD_ENUM_OP_NOT(E)
} // namespace ARLib