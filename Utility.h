#pragma once
#include "BaseTraits.h"

namespace ARLib {

    template <class T>
    inline constexpr T&& Forward(typename RemoveReference<T>::type& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    template <class T>
    inline constexpr T&& Forward(typename RemoveReference<T>::type&& t) noexcept
    {
        return static_cast<T&&>(t);
    }

    template<typename T>
    constexpr RemoveReferenceT<T>&& move(T&& t) noexcept
    {
        return static_cast<RemoveReferenceT<T>&&>(t);
    }
}