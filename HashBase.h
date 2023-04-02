#pragma once
#define HASHBASE_INCLUDE
#include "Compat.h"
#include "TypeTraits.h"
namespace ARLib {

#ifdef ENVIRON64
constexpr inline size_t offset_basis = 14695981039346656037ULL;
constexpr inline size_t fn_prime     = 1099511628211ULL;
#else
constexpr inline size_t offset_basis = 2166136261U;
constexpr inline size_t fn_prime     = 16777619U;
#endif
[[nodiscard]] size_t murmur_hash_bytes(const char* const ptr, size_t count, size_t seed);
[[nodiscard]] inline size_t fn_append_bytes(size_t val, const unsigned char* const first, const size_t count) noexcept {
    for (size_t i = 0; i < count; ++i) {
        val ^= static_cast<size_t>(first[i]);
        val *= fn_prime;
    }
    return val;
}
template <class Key>
[[nodiscard]] size_t fnv_append_value(const size_t val, const Key& key) noexcept {
    static_assert(IsTrivialV<Key>, "Only trivial types can be directly hashed.");
    return fn_append_bytes(val, &reinterpret_cast<const unsigned char&>(key), sizeof(Key));
}
template <class Key>
[[nodiscard]] size_t hash_representation(const Key& key) noexcept {
    return fnv_append_value(offset_basis, key);
}
template <class Key>
[[nodiscard]] size_t hash_array_representation(const Key* const first, const size_t count) noexcept {
    static_assert(IsTrivialV<Key>, "Only trivial types can be directly hashed.");
    return fn_append_bytes(offset_basis, reinterpret_cast<const unsigned char*>(first), count * sizeof(Key));
}
uint32_t hash_bswap(uint32_t value);
template <class Key>
[[nodiscard]] size_t hash_integral_fast(const Key& k) noexcept {
    static_assert(IsIntegralV<Key>, "Only integer types may call this function");
    #ifdef ON_WINDOWS
    return static_cast<size_t>(k ^ hash_bswap(static_cast<uint32_t>(k * 1086221891)));
    #else
    return static_cast<size_t>(k ^ __builtin_bswap32(static_cast<uint32_t>(k * 1086221891)));
    #endif
}

template <class Key>
struct Hash;
template <class Key, bool Enabled>
struct ConditionallyEnabledHash {
    [[nodiscard]] size_t operator()(const Key& key) const noexcept(noexcept(Hash<Key>::do_hash(key))) {
        return Hash<Key>::do_hash(key);
    }
};
template <class Key>
struct ConditionallyEnabledHash<Key, false> {    // conditionally disabled hash base
    static_assert(
    AlwaysFalse<Key>,
    "Hash{map|table} key types need to have a template specialization of the struct "
    "ARLib::Hash<T> to be allowed as keys, such struct template specialization must have an operator() method "
    "that takes a const ref of type T and returns a size_t value representing the hash. "
    "The key type must also have an available equality operator."
    );
    ConditionallyEnabledHash()                                           = delete;
    ConditionallyEnabledHash(const ConditionallyEnabledHash&)            = delete;
    ConditionallyEnabledHash(ConditionallyEnabledHash&&)                 = delete;
    ConditionallyEnabledHash& operator=(const ConditionallyEnabledHash&) = delete;
    ConditionallyEnabledHash& operator=(ConditionallyEnabledHash&&)      = delete;
};
template <class Key>
struct Hash :
    ConditionallyEnabledHash<
    Key, !IsConstV<Key> && !IsVolatileV<Key> && (IsEnumV<Key> || IsIntegralV<Key> || IsPointerV<Key>)> {
    static size_t do_hash(const Key& key) noexcept {
        if constexpr (IsIntegralV<Key>)
            return hash_integral_fast(key);
        else if constexpr (IsEnumV<Key>)
            return hash_integral_fast(static_cast<UnderlyingTypeT<Key>>(key));
        return hash_representation(key);
    }
};
template <>
struct Hash<float> {
    [[nodiscard]] size_t operator()(const float key) const noexcept {
        return hash_representation(key == 0.0F ? 0.0F : key);
    }
};
template <>
struct Hash<double> {
    [[nodiscard]] size_t operator()(const double key) const noexcept {
        return hash_representation(key == 0.0 ? 0.0 : key);    // map -0 to 0
    }
};
template <>
struct Hash<long double> {
    [[nodiscard]] size_t operator()(const long double key) const noexcept {
        return hash_representation(key == 0.0L ? 0.0L : key);    // map -0 to 0
    }
};
template <>
struct Hash<nullptr_t> {
    [[nodiscard]] size_t operator()(nullptr_t) const noexcept {
        void* null{};
        return hash_representation(null);
    }
};
template <size_t N>
struct Hash<char[N]> {
    [[nodiscard]] size_t operator()(const char (&key)[N]) const noexcept { 
        if constexpr (windows_build) {
            // this hash yields way better code on windows
            return hash_array_representation(key, N);
        } else {
            constexpr size_t seed = static_cast<size_t>(0xc70f6907UL);
            return murmur_hash_bytes(key, N, seed);
        }
    }
};
}    // namespace ARLib
