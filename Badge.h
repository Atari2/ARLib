#pragma once

namespace ARLib {
    template <typename T>
    class Badge {
        friend T;
        constexpr Badge() = default;
        constexpr Badge(const Badge&) = default;
        constexpr Badge(Badge&&)  noexcept = default;
        constexpr Badge<T>& operator=(const Badge<T>&) = default;
        constexpr Badge<T>& operator=(Badge<T>&&)  noexcept = default;
    };
} // namespace ARLib
