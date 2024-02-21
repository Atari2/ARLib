#pragma once
#include "Concepts.hpp"
namespace ARLib {
template <typename T>
struct TagType {};
namespace EnumHelpers {

    template <typename T>
    struct EnumArrayProvider {
        static void construct_enum_array(TagType<T>);
    };

    template <typename T>
    concept HasEnumArrayProvider = Enum<T> && requires {
        { EnumArrayProvider<T>::construct_enum_array(TagType<T>{}) } -> NonVoid;
    };

    template <typename T>
    requires(Enum<T> && HasEnumArrayProvider<T>)
    struct EnumMapProvider;

    template <typename T>
    concept EnumSupportsMap = Enum<T> && requires {
        { EnumMapProvider<T>{} };
    };
}    // namespace EnumHelpers
template <typename Enum>
concept FancyEnum = EnumHelpers::EnumSupportsMap<Enum>;
}    // namespace ARLib