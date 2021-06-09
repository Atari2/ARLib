#pragma once
#define HASHBASE_INCLUDE
#include "Compat.h"
#include "TypeTraits.h"

namespace ARLib {

#ifdef ENVIRON64
    inline constexpr size_t offset_basis = 14695981039346656037ULL;
    inline constexpr size_t fn_prime = 1099511628211ULL;
#else
    inline constexpr size_t offset_basis = 2166136261U;
    inline constexpr size_t fn_prime = 16777619U;
#endif

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

    template <class Key>
    struct Hash;

    template <class Key, bool Enabled>
    struct ConditionallyEnabledHash {
        [[nodiscard]] size_t operator()(const Key& key) const noexcept(noexcept(Hash<Key>::do_hash(key))) {
            return Hash<Key>::do_hash(key);
        }
    };

    template <class Key>
    struct ConditionallyEnabledHash<Key, false> { // conditionally disabled hash base
        ConditionallyEnabledHash() = delete;
        ConditionallyEnabledHash(const ConditionallyEnabledHash&) = delete;
        ConditionallyEnabledHash(ConditionallyEnabledHash&&) = delete;
        ConditionallyEnabledHash& operator=(const ConditionallyEnabledHash&) = delete;
        ConditionallyEnabledHash& operator=(ConditionallyEnabledHash&&) = delete;
    };

    template <class Key>
    struct Hash : ConditionallyEnabledHash<Key, !IsConstV<Key> && !IsVolatileV<Key> && (IsEnumV<Key> || IsIntegralV<Key> || IsPointerV<Key>)> {
        static size_t do_hash(const Key& key) noexcept {
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
            return hash_representation(key == 0.0 ? 0.0 : key); // map -0 to 0
        }
    };

    template <>
    struct Hash<long double> {
        [[nodiscard]] size_t operator()(const long double key) const noexcept {
            return hash_representation(key == 0.0L ? 0.0L : key); // map -0 to 0
        }
    };

    template <>
    struct Hash<nullptr_t> {
        [[nodiscard]] size_t operator()(nullptr_t) const noexcept {
            void* null{};
            return hash_representation(null);
        }
    };
}
