#pragma once
#include "Algorithm.hpp"
#include "Array.hpp"
#include "CharConv.hpp"
#include "Concepts.hpp"
#include "Conversion.hpp"
#include "Pair.hpp"
#include "PrintInfo.hpp"
#include "StringView.hpp"
#include "GenericView.hpp"
#include "Optional.hpp"
#include "EnumConcepts.hpp"
namespace ARLib {
template <Enum T>
constexpr auto ToUnderlying(T value) {
    return static_cast<UnderlyingTypeT<T>>(value);
}
template <Enum T>
constexpr auto UpCast(auto value) {
    return static_cast<T>(value);
}
namespace EnumHelpers {
    template <Enum V, size_t N>
    class EnumStrHashMap {
        // round up to power of 2 to avoid modulos and just use bitwise ops
        constexpr static size_t ACTUAL_SIZE = BitCeil(N);
        constexpr static size_t FOR_MOD_OPS = ACTUAL_SIZE - 1;
        struct KeyBkt {
            V key{};
            bool used{};
        };
        Array<Pair<KeyBkt, StringView>, ACTUAL_SIZE> m_internal_map;
        size_t m_size = 0;

        public:
        constexpr bool insert(V key, StringView val) {
            if (m_size == m_internal_map.size()) return false;
            size_t idx = static_cast<size_t>(from_enum(key)) & FOR_MOD_OPS;
            while (m_internal_map[idx].first().used) { idx = (idx + 1) & FOR_MOD_OPS; }
            m_internal_map[idx] = Pair{
                KeyBkt{move(key), true},
                val
            };
            m_size++;
            return true;
        }
        constexpr StringView operator[](V key) const {
            size_t idx     = static_cast<size_t>(from_enum(key)) & FOR_MOD_OPS;
            size_t n_tries = 0;
            while (m_internal_map[idx].first().key != key) {
                idx = (idx + 1) & FOR_MOD_OPS;
                if (n_tries++ == m_size) return "InvalidEnumValue"_sv;
            }
            return m_internal_map[idx].second();
        }
    };
    constexpr size_t count_enum_values(StringView view) {
        size_t count = 1;
        for (const auto c : view) count += (c == ',');
        return count;
    }
    template <Enum T>
    constexpr StringView get_enum_full_string(TagType<T>) {
        return "";
    }
    template <Enum T>
    requires(!get_enum_full_string<T>({}).empty())
    struct EnumArrayProvider<T> {
        constexpr static auto construct_enum_array(TagType<T>) {
            constexpr StringView view = get_enum_full_string<T>({});
            Array<Pair<StringView, T>, count_enum_values(view)> enum_map_l;
            UnderlyingTypeT<T> current_val{};
            auto get_pair = [&view, &current_val](size_t first_idx, size_t second_idx) {
                constexpr bool Signed = IsSigned<UnderlyingTypeT<T>>;
                auto sub              = view.substringview_fromlen(first_idx, second_idx - first_idx);
                auto equal_idx        = sub.index_of('=');
                if (equal_idx == StringView::npos) {
                    // no equal, get next value
                    auto first_nospace_idx = sub.index_not_of(' ');
                    auto last_nospace_idx  = sub.index_of(' ', first_nospace_idx);
                    return Pair{ sub.substringview_fromlen(first_nospace_idx, last_nospace_idx),
                                 to_enum<T>(current_val++) };
                } else {
                    auto first_nospace_idx = sub.index_not_of(' ');
                    auto last_nospace_idx  = sub.index_of(' ', first_nospace_idx);
                    using Ut               = UnderlyingTypeT<T>;
                    const auto value       = to_enum<T>(
                    Signed ? static_cast<Ut>(cxpr::StrViewToI64(sub.substringview_fromlen(equal_idx + 1))) :
                                   static_cast<Ut>(cxpr::StrViewToU64(sub.substringview_fromlen(equal_idx + 1)))
                    );
                    current_val = from_enum(value) + UnderlyingTypeT<T>{ 1 };
                    return Pair{ sub.substringview_fromlen(
                                 first_nospace_idx,
                                 (last_nospace_idx < equal_idx ? last_nospace_idx : equal_idx) - first_nospace_idx
                                 ),
                                 value };
                }
            };
            size_t first_idx  = 0;
            size_t second_idx = view.index_of(',');
            size_t i          = 0;
            while (second_idx != StringView::npos) {
                enum_map_l[i++] = get_pair(first_idx, second_idx);
                first_idx       = second_idx + 1;
                second_idx      = view.index_of(',', first_idx);
            }
            enum_map_l[i] = get_pair(first_idx, view.size());
            return enum_map_l;
        }
    };
    template <Enum T>
    constexpr auto
    construct_enum_map(const Array<Pair<StringView, T>, count_enum_values(get_enum_full_string<T>({}))>& enum_array) {
        constexpr size_t sz = count_enum_values(get_enum_full_string<T>({}));
        EnumStrHashMap<T, sz> map;
        for (const auto& [k, v] : enum_array) {
            const bool inserted = map.insert(v, k);
            if (is_constant_evaluated()) {
                // this allocation is here for the same reason as the one at line 178
                if (!inserted) { new int[static_cast<size_t>(from_enum(v))]; }
            }
        }
        return map;
    }
    template <typename T>
    requires(Enum<T> && HasEnumArrayProvider<T>)
    struct EnumMapProvider {
        constexpr static auto enum_array = EnumArrayProvider<T>::construct_enum_array({});
        constexpr static auto map        = construct_enum_map<T>(enum_array);
    };
    template <EnumSupportsMap T>
    class EnumIterator {
        size_t index                            = 0;
        constexpr static const auto& s_enum_map = EnumMapProvider<T>::enum_array;

        public:
        constexpr EnumIterator() = default;
        constexpr EnumIterator(size_t idx) : index(idx) {}
        constexpr const T& operator*() const { return s_enum_map[index].second(); }
        constexpr const T& operator->() const { return s_enum_map[index].second(); }
        constexpr bool operator==(const EnumIterator& other) const { return index == other.index; }
        constexpr bool operator!=(const EnumIterator& other) const { return index != other.index; }
        constexpr EnumIterator& operator++() {
            index++;
            return *this;
        }
        constexpr EnumIterator operator++(int) {
            EnumIterator iter = *this;
            index++;
            return iter;
        }
        constexpr EnumIterator& operator--() {
            index--;
            return *this;
        }
        constexpr EnumIterator operator--(int) {
            EnumIterator iter = *this;
            index--;
            return iter;
        }
        constexpr size_t operator-(const EnumIterator& other) const { return index - other.index; }
    };
}    // namespace EnumHelpers

enum class EnumStrSearchType { HashMap, Linear };
template <EnumStrSearchType SearchType = EnumStrSearchType::HashMap, EnumHelpers::EnumSupportsMap T>
constexpr StringView enum_to_str_view(T e) {
    if constexpr (SearchType == EnumStrSearchType::Linear) {
        constexpr const auto& enum_map = EnumHelpers::EnumMapProvider<T>::enum_array;
        auto idx                       = find_if(enum_map, [&e](const auto& p) {
            const auto& [name, val] = p;
            return val == e;
        });
        if (idx != npos_) return enum_map[idx].first();
        if (is_constant_evaluated()) {
            // this allocation is here to stop compilation
            // if your code reached this allocation it means you passed an invalid value to enum_to_str()
            // this *does not* cause leaks because it only gets executed during constexpr evaluation
            // and during constexpr evaluation allocations which are not freed in the same scope cause a compilation
            // failure because the expression is not constant anymore for example, the error msvc gives you is:
            // `error C2131: expression did not evaluate to a constant`
            // `Common.h(341): note: failure was caused by allocated storage not being deallocated`
            new int[from_enum(e)];
        }
        return "invalid enum value";
    } else {
        // I'm not sure if this is better than linear search, it's really just a poor's man linear probing
        // hash map with O(1) access time (if no collisions)
        // and O(N) max round trip. It will throw if the enum isn't in the map
        constexpr const auto& enum_map = EnumHelpers::EnumMapProvider<T>::map;
        return enum_map[e];
    }
}
template <EnumHelpers::EnumSupportsMap T>
constexpr size_t enum_size() {
    constexpr const auto& enum_array = EnumHelpers::EnumMapProvider<T>::enum_array;
    return enum_array.size();
}
template <EnumStrSearchType SearchType = EnumStrSearchType::HashMap, EnumHelpers::EnumSupportsMap T>
String enum_to_str(T e) {
    return String{ enum_to_str_view<SearchType, T>(e) };
}
template <EnumHelpers::EnumSupportsMap T>
Optional<T> enum_parse(StringView v) {
    constexpr const auto& enum_map = EnumHelpers::EnumMapProvider<T>::enum_array;
    return enum_map.iter().find_if([v](const auto& p) { return p.first() == v; }).map([](auto&& p) {
        return p.second();
    });
}
template <EnumHelpers::EnumSupportsMap T>
struct ForEachEnum {
    constexpr auto begin() const { return EnumHelpers::EnumIterator<T>{ 0 }; }
    constexpr auto end() const {
        return EnumHelpers::EnumIterator<T>{ EnumHelpers::EnumMapProvider<T>::enum_array.size() };
    }
    constexpr auto iter() const { return IteratorView{ *this }; }
    constexpr auto size() const { return EnumHelpers::EnumMapProvider<T>::enum_array.size(); }
};
template <EnumHelpers::EnumSupportsMap T>
constexpr auto for_each_enum() {
    return ForEachEnum<T>{};
}
template <EnumHelpers::EnumSupportsMap T, typename Functor>
requires(requires { declval<Functor>()(declval<T>()); })
constexpr auto for_each_enum(Functor func) {
    using Res = InvokeResultT<Functor, T>;
    if constexpr (IsVoid<Res>::value) {
        for (auto val : ForEachEnum<T>{}) { func(val); }
    } else {
        Array<Res, EnumHelpers::EnumMapProvider<T>::enum_array.size()> result_array;
        size_t i = 0;
        for (auto val : ForEachEnum<T>{}) { result_array[i++] = func(val); }
        return result_array;
    }
}
template <EnumHelpers::EnumSupportsMap E>
struct PrintInfo<E> {
    E m_val;
    PrintInfo(E val) : m_val(val) {}
    String repr() const { return enum_to_str(m_val); }
};
}    // namespace ARLib
#define ENUM_TO_STR(en, ...)                                                                                           \
    template <>                                                                                                        \
    constexpr StringView ARLib::EnumHelpers::get_enum_full_string(TagType<en>) {                                       \
        return #__VA_ARGS__;                                                                                           \
    }

#define MAKE_FANCY_ENUM(en, ut, ...)                                                                                   \
    enum class en : ut { __VA_ARGS__ };                                                                                \
    ENUM_TO_STR(en, __VA_ARGS__)

#define BITFIELD_ENUM_OP_OR(E)                                                                                         \
    constexpr auto operator|(E self, E other) {                                                                        \
        return UpCast<E>(ToUnderlying(self) | ToUnderlying(other));                                                    \
    }
#define BITFIELD_ENUM_OP_AND(E)                                                                                        \
    constexpr auto operator&(E self, E other) {                                                                        \
        return UpCast<E>(ToUnderlying(self) & ToUnderlying(other));                                                    \
    }

#define BITFIELD_ENUM_OP_NONE(E)                                                                                       \
    constexpr auto operator!(E self) {                                                                                 \
        return self == E::None;                                                                                        \
    }
#define BITFIELD_ENUM_OP_LOG_AND(E)                                                                                    \
    constexpr auto operator&&(E self, E other) {                                                                       \
        return ToUnderlying(self) && ToUnderlying(other);                                                              \
    }
#define BITFIELD_ENUM_OP_LOG_OR(E)                                                                                     \
    constexpr auto operator||(E self, E other) {                                                                       \
        return ToUnderlying(self) || ToUnderlying(other);                                                              \
    }
#define BITFIELD_ENUM_OP_XOR(E)                                                                                        \
    constexpr auto operator^(E self, E other) {                                                                        \
        return UpCast<E>(ToUnderlying(self) ^ ToUnderlying(other));                                                    \
    }
#define BITFIELD_ENUM_OP_NOT(E)                                                                                        \
    constexpr auto operator~(E self) {                                                                                 \
        return UpCast<E>(~ToUnderlying(self));                                                                         \
    }

#define MAKE_BITFIELD_ENUM(E)                                                                                          \
    BITFIELD_ENUM_OP_OR(E)                                                                                             \
    BITFIELD_ENUM_OP_AND(E)                                                                                            \
    BITFIELD_ENUM_OP_LOG_AND(E)                                                                                        \
    BITFIELD_ENUM_OP_LOG_OR(E)                                                                                         \
    BITFIELD_ENUM_OP_XOR(E)                                                                                            \
    BITFIELD_ENUM_OP_NOT(E)
