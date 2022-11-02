#pragma once
#include "Array.h"
#include "Concepts.h"
#include "Hash.h"
#include "StringView.h"
#include "Utility.h"

namespace ARLib {
    namespace cxpr {
        using crc32 = HashAlgorithm<HashType::CRC32>;
        constexpr size_t test = crc32::calculate("stack-overflow"_sv);
        static_assert(test == 0x335CC04A, "Testing crc32 produced wrong result");
        template <typename T>
        struct Hasher {
            static_assert(AlwaysFalse<T>, "Implement the hasher class for type T");
        };
        template <typename T>
        concept Hashable = requires {
            { std::declval<Hasher<T>>()(std::declval<T>()) } -> SameAs<uint32_t>;
        };
        template <>
        struct Hasher<StringView> {
            constexpr uint32_t operator()(const StringView str) const { return crc32::calculate(str); }
        };
        template <Hashable V, size_t SZ, size_t BASE_SIZE = 255>
        requires(DefaultConstructible<V>&& EqualityComparable<V>) class HashTable {
            constexpr static auto MIN_SIZE = BitCeil(SZ) - 1;
            constexpr static auto FOR_MOD_OPS = (MIN_SIZE < BASE_SIZE) ? BASE_SIZE : MIN_SIZE;
            constexpr static auto hasher = Hasher<V>{};
            struct Bucket {
                V val;
                uint32_t hash;
                bool used;
            };
            Array<Bucket, FOR_MOD_OPS + 1> m_table{};

            public:
            constexpr void insert(V val) {
                const uint32_t initial_hash = hasher(val);
                size_t index = initial_hash & FOR_MOD_OPS;
                while (m_table[index].used) {
                    index = (index + 1) & FOR_MOD_OPS;
                }
                m_table[index] = Bucket{val, initial_hash, true};
            }
            constexpr bool contains(V val) const {
                const uint32_t initial_hash = hasher(val);
                size_t index = initial_hash & FOR_MOD_OPS;
                while (m_table[index].used) {
                    if (m_table[index].hash == initial_hash) {
                        if (m_table[index].val == val) return true;
                    }
                    index = (index + 1) & FOR_MOD_OPS;
                }
                return false;
            }
            constexpr auto find(V val) const {
                const uint32_t initial_hash = hasher(val);
                size_t index = initial_hash & FOR_MOD_OPS;
                while (m_table[index].used) {
                    if (m_table[index].hash == initial_hash) {
                        if (m_table[index].val == val) return m_table.begin() + index;
                    }
                    index = (index + 1) & FOR_MOD_OPS;
                }
                return m_table.end();
            }
            template <typename Functor, Hashable T>
            requires(SameAs<InvokeResultT<Functor, V, T>, bool>) constexpr auto find(T val, Functor cmp) const {
                constexpr Hasher<T> h{};
                const uint32_t initial_hash = h(val);
                size_t index = initial_hash & FOR_MOD_OPS;
                while (m_table[index].used) {
                    if (m_table[index].hash == initial_hash) {
                        if (cmp(m_table[index].val, val)) return m_table.begin() + index;
                    }
                    index = (index + 1) & FOR_MOD_OPS;
                }
                return m_table.end();
            }
            constexpr auto end() const { return m_table.end(); }
            constexpr auto begin() const { return m_table.begin(); }
        };
    } // namespace cxpr
} // namespace ARLib