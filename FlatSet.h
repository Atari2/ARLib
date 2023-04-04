#pragma once

#include "HashBase.h"
#include "Concepts.h"
#include "Vector.h"
#include "Printer.h"
#include "Array.h"
#include "EnumHelpers.h"
/*
A FlatSet implementation with a design very similar to that of abseil's swiss tables, albeit simplified for the sake of complexity
from here: https://github.com/abseil/abseil-cpp/blob/master/absl/container/internal/raw_hash_set.h
The BitMask class is a simplified version of abseil's bitmask
which can be found here: https://github.com/abseil/abseil-cpp/blob/5102fe1680a629b052add85dc46c3be04063be67/absl/container/internal/raw_hash_set.h#L405
design notes/algorithms taken from:
- https://www.youtube.com/watch?v=ncHmEUmJZf4
- https://www.youtube.com/watch?v=JZE3_0qvrMg
- https://abseil.io/about/design/swisstables
*/

namespace ARLib {
namespace internal {
    uint32_t trailing_zeros(uint32_t val);
}
template <typename T>
class BitMask {
    T m_mask;
    public:
    explicit operator bool() const { return m_mask != 0; }
    explicit constexpr BitMask(T mask) : m_mask(mask) {}
    BitMask& operator++() {
        m_mask &= (m_mask - 1);
        return *this;
    }
    BitMask begin() const { return *this; }
    BitMask end() const { return BitMask{ 0 }; };
    uint32_t lowest_bit_set() const { return static_cast<uint32_t>(internal::trailing_zeros(m_mask)); }
    uint32_t operator*() const { return lowest_bit_set(); }
    private:
    friend bool operator==(const BitMask& a, const BitMask& b) { return a.m_mask == b.m_mask; }
    friend bool operator!=(const BitMask& a, const BitMask& b) { return a.m_mask != b.m_mask; }
};
namespace internal {
    constexpr static inline size_t flatset_bucket_size = 16;
    enum class Control : int8_t { Empty = -128, Deleted = -2 };
    using MetadataBlock = ARLib::Array<Control, flatset_bucket_size>;
    BitMask<uint32_t> match(int8_t hash, const MetadataBlock& block);
    BitMask<uint32_t> match_empty(const MetadataBlock& block);
    BitMask<uint32_t> match_non_empty(const MetadataBlock& block);
}    // namespace internal
template <typename T, typename HashCls>
requires Hashable<T, HashCls>
class FlatSet;
template <typename T, typename HashCls>
requires Hashable<T, HashCls>
class FlatSetIterator {
    friend FlatSet<T, HashCls>;
    const FlatSet<T, HashCls>* m_set;
    size_t m_current_bucket = 0;
    BitMask<uint32_t> m_current_item{ 0 };
    friend FlatSet<T, HashCls>;
    public:
    FlatSetIterator(const FlatSet<T, HashCls>* set, size_t bucket, BitMask<uint32_t> item) :
        m_set{ set }, m_current_bucket{ bucket }, m_current_item{ item } {}
    const T& operator*() const;
    bool operator==(const FlatSetIterator& other) const {
        return m_set == other.m_set && m_current_bucket == other.m_current_bucket &&
               m_current_item == other.m_current_item;
    }
    FlatSetIterator operator++(int);
    FlatSetIterator& operator++();
    FlatSetIterator operator--(int);
    FlatSetIterator& operator--();
};
template <typename T>
struct FlatSetStorage {
    constexpr static inline size_t ObjectSize  = sizeof(T) + (sizeof(T) % alignof(T));
    constexpr static inline size_t StorageSize = ObjectSize * internal::flatset_bucket_size;
    // we do not need to value initialize the storage
    // since we call new (mem) T on it and that doesn't need memory to be zerod.
    alignas(T) uint8_t storage[StorageSize];
    uint16_t initialized_mask{ 0 };
    FlatSetStorage()                            = default;
    FlatSetStorage(const FlatSetStorage& other) = delete;
    FlatSetStorage(FlatSetStorage&& other) noexcept {
        for (auto bit : BitMask{ other.initialized_mask }) {
            initialize_at(bit, move(other.at(bit)));
            other.destroy_at(bit);
        }
    }
    FlatSetStorage& operator=(const FlatSetStorage& other) = delete;
    FlatSetStorage& operator=(FlatSetStorage&& other) noexcept {
        for (auto bit : BitMask{ initialized_mask }) { destroy_at(bit); }
        for (auto bit : BitMask{ other.initialized_mask }) {
            initialize_at(bit, move(other.at(bit)));
            other.destroy_at(bit);
        }
        return *this;
    }
    T& initialize_at(size_t index, T&& value) {
        initialized_mask |= (1 << index);
        uint8_t* obj_ptr = &storage[ObjectSize * index];
        T* obj           = new (obj_ptr) T{ move(value) };
        return *obj;
    }
    ConstIterator<T> begin() const { return ConstIterator<T>{ reinterpret_cast<const T*>(storage) }; }
    ConstIterator<T> end() const {
        return ConstIterator<T>{ reinterpret_cast<const T*>(storage) + (sizeof(T) * internal::flatset_bucket_size) };
    }
    Iterator<T> begin() { return Iterator{ reinterpret_cast<T*>(storage) }; }
    Iterator<T> end() {
        return Iterator{ reinterpret_cast<T*>(storage) + (sizeof(T) * internal::flatset_bucket_size) };
    }
    void destroy_at(size_t index) {
        initialized_mask &= ~(1 << index);
        T& obj = *reinterpret_cast<T*>(&storage[ObjectSize * index]);
        obj.~T();
    }
    T& at(size_t index) { return *reinterpret_cast<T*>(&storage[ObjectSize * index]); }
    const T& at(size_t index) const { return *reinterpret_cast<const T*>(&storage[ObjectSize * index]); }
    ~FlatSetStorage() {
        for (auto bit : BitMask{ initialized_mask }) { destroy_at(bit); }
    }
};
template <typename T, typename HashCls = Hash<T>>
requires Hashable<T, HashCls>
class FlatSet {
    friend FlatSetIterator<T, HashCls>;
    using Control                                    = internal::Control;
    using MetadataBlock                              = internal::MetadataBlock;
    constexpr static inline size_t bucket_size       = internal::flatset_bucket_size;
    constexpr static inline size_t base_buckets      = 16;
    constexpr static inline double s_max_load_factor = 7.0 / 8.0;
    constexpr static size_t h1(size_t full_value) { return (full_value >> 7); };
    constexpr static int8_t h2(size_t full_value) { return full_value & 0x7f; }
    struct Bucket {
        MetadataBlock m_ctrl_block{ Control::Empty, Control::Empty, Control::Empty, Control::Empty,
                                    Control::Empty, Control::Empty, Control::Empty, Control::Empty,
                                    Control::Empty, Control::Empty, Control::Empty, Control::Empty,
                                    Control::Empty, Control::Empty, Control::Empty, Control::Empty };
        FlatSetStorage<T> m_bucket{};
        Bucket() = default;
        Bucket(Bucket&& other) noexcept : m_ctrl_block{ move(other.m_ctrl_block) }, m_bucket{ move(other.m_bucket) } {
            for (size_t i = 0; i < m_ctrl_block.size(); ++i) { other.m_ctrl_block[i] = Control::Empty; }
        }
        Bucket& operator=(Bucket&& other) noexcept {
            m_ctrl_block = move(other.m_ctrl_block);
            m_bucket     = move(other.m_bucket);
            for (size_t i = 0; i < m_ctrl_block.size(); ++i) { other.m_ctrl_block[i] = Control::Empty; }
            return *this;
        }
    };
    using BucketVec = Vector<Bucket>;
    BucketVec m_buckets{};
    HashCls m_hasher{};
    size_t m_size = 0;
    bool needs_rehash() const { return capacity() == 0 ? false : load_factor() >= max_load_factor(); }
    auto prepare_for_insert(const T& value) {
        if (needs_rehash()) rehash();
        const size_t num_groups = m_buckets.size();
        const size_t hash       = m_hasher(value);
        size_t group            = h1(hash) % num_groups;
        while (true) {
            Bucket& b       = m_buckets[group];
            const auto mask = internal::match(h2(hash), b.m_ctrl_block);
            for (auto it = mask.begin(); it != mask.end(); ++it) {
                auto bit = *it;
                // we found an h2 hash match, check if we're trying to insert the same value
                // that's already present, if so, do nothing and return
                if (b.m_bucket.at(bit) == value)
                    return Pair{
                        false, FlatSetIterator<T, HashCls>{this, group, it}
                    };
            }
            if (auto m = internal::match_empty(b.m_ctrl_block); m) {
                auto index            = *m;
                b.m_ctrl_block[index] = to_enum<Control>(0_i8 | h2(hash));
                m_size++;
                return Pair{
                    true, FlatSetIterator<T, HashCls>{this, group, m}
                };
            };
            group = (group + 1) % num_groups;
        }
    }

    public:
    FlatSet() { m_buckets.resize(base_buckets); };
    size_t capacity() const { return m_buckets.size() * bucket_size; }
    size_t size() const { return m_size; }
    double max_load_factor() const { return s_max_load_factor; }
    double load_factor() const {
        size_t cap = capacity();
        if (cap == 0) return 0.0;
        return static_cast<double>(size()) / static_cast<double>(cap);
    }
    auto rehash() {
        size_t needed_buckets = bit_round_growth(m_buckets.size() + 1);
        BucketVec buckets{ move(m_buckets) };
        m_buckets.resize(needed_buckets);
        m_size = 0;
        for (auto& b : buckets) {
            for (auto bit : internal::match_non_empty(b.m_ctrl_block)) { insert(move(b.m_bucket.at(bit))); }
        }
    }
    auto clear() {
        m_buckets.clear_retain();
        m_buckets.resize(base_buckets);
        m_size = 0;
    }
    auto find(const T& value) const {
        const size_t num_groups = m_buckets.size();
        const size_t hash       = m_hasher(value);
        size_t group            = h1(hash) % num_groups;
        while (true) {
            const Bucket& b = m_buckets[group];
            const auto mask = internal::match(h2(hash), b.m_ctrl_block);
            for (auto it = mask.begin(); it != mask.end(); ++it) {
                auto bit = *it;
                if (value == b.m_bucket.at(bit)) { return FlatSetIterator<T, HashCls>{ this, group, it }; }
            }
            if (internal::match_empty(b.m_ctrl_block)) return end();
            group = (group + 1) % num_groups;
        }
    }
    template <Hashable O>
    requires EqualityComparableWith<O, T>
    auto find(const O& value) const {
        const auto hasher       = Hash<O>{};
        const size_t num_groups = m_buckets.size();
        const size_t hash       = hasher(value);
        size_t group            = h1(hash) % num_groups;
        while (true) {
            const Bucket& b = m_buckets[group];
            const auto mask = internal::match(h2(hash), b.m_ctrl_block);
            for (auto it = mask.begin(); it != mask.end(); ++it) {
                auto bit = *it;
                if (value == b.m_bucket.at(bit)) { return FlatSetIterator<T, HashCls>{ this, group, it }; }
            }
            if (internal::match_empty(b.m_ctrl_block)) return end();
            group = (group + 1) % num_groups;
        }
    }
    auto begin() const {
        for (size_t i = 0; i < m_buckets.size(); ++i) {
            auto mask = internal::match_non_empty(m_buckets[i].m_ctrl_block);
            if (mask != BitMask{ 0_u32 }) { return FlatSetIterator<T, HashCls>{ this, i, mask }; }
        }
        return end();
    }
    auto end() const { return FlatSetIterator<T, HashCls>{ this, m_buckets.size(), BitMask{ 0_u32 } }; }
    bool contains(const T& value) const { return find(value) != end(); }
    template <Hashable O>
    requires EqualityComparableWith<O, T>
    auto remove(const O& value) {
        const auto hasher       = Hash<O>{};
        const size_t num_groups = m_buckets.size();
        const size_t hash       = hasher(value);
        size_t group            = h1(hash) % num_groups;
        while (true) {
            Bucket& b       = m_buckets[group];
            const auto mask = internal::match(h2(hash), b.m_ctrl_block);
            for (auto bit : mask) {
                if (b.m_bucket.at(bit) == value) {
                    b.m_bucket.destroy_at(bit);
                    b.m_ctrl_block[bit] = Control::Deleted;
                    m_size--;
                    return true;
                };
            }
            if (internal::match_empty(b.m_ctrl_block)) return false;
            group = (group + 1) % num_groups;
        }
    }
    bool remove(const T& value) {
        const size_t num_groups = m_buckets.size();
        const size_t hash       = m_hasher(value);
        size_t group            = h1(hash) % num_groups;
        while (true) {
            Bucket& b       = m_buckets[group];
            const auto mask = internal::match(h2(hash), b.m_ctrl_block);
            for (auto bit : mask) {
                if (b.m_bucket.at(bit) == value) {
                    b.m_bucket.destroy_at(bit);
                    b.m_ctrl_block[bit] = Control::Deleted;
                    m_size--;
                    return true;
                };
            }
            if (internal::match_empty(b.m_ctrl_block)) return false;
            group = (group + 1) % num_groups;
        }
    }
    bool insert(T&& value) {
        auto&& [ins, it] = prepare_for_insert(value);
        m_buckets[it.m_current_bucket].m_bucket.initialize_at(*it.m_current_item, Forward<T>(value));
        return ins;
    }
};
template <typename T, typename HashCls>
requires Hashable<T, HashCls>
const T& FlatSetIterator<T, HashCls>::operator*() const {
    return m_set->m_buckets[m_current_bucket].m_bucket.at(*m_current_item);
}
template <typename T, typename HashCls>
requires Hashable<T, HashCls>
FlatSetIterator<T, HashCls>& FlatSetIterator<T, HashCls>::operator++() {
    auto end = BitMask{ 0_u32 };
    ++m_current_item;
    while (m_current_item == end) {
        ++m_current_bucket;
        if (m_current_bucket == m_set->m_buckets.size()) return *this;    // we're at end
        // reset item mask
        m_current_item = internal::match_non_empty(m_set->m_buckets[m_current_bucket].m_ctrl_block);
    }
    return *this;
}
template <typename T, typename HashCls>
requires Hashable<T, HashCls>
FlatSetIterator<T, HashCls> FlatSetIterator<T, HashCls>::operator++(int) {
    FlatSetIterator copy{ *this };
    this->operator++();
    return copy;
}
}    // namespace ARLib