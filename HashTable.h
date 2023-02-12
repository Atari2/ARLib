#pragma once
#include "Algorithm.h"
#include "Assertion.h"
#include "Concepts.h"
#include "HashBase.h"
#include "Vector.h"
#include "cmath_compat.h"
namespace ARLib {
enum class InsertionResult { New, Replace };
enum class DeletionResult { Success, Failure };

template <typename T>
using HashTableStorage = Vector<Vector<T>>;
template <typename T>
class HashTableIterator {
    HashTableStorage<T>& m_backing_store;
    size_t m_current_bucket       = 0;
    size_t m_current_vector_index = 0;
    size_t m_bkt_size             = 0;

    public:
    using ValueType              = T;
    constexpr static size_t npos = static_cast<size_t>(-1);
    explicit HashTableIterator(HashTableStorage<T>& backing_store) :
        m_backing_store(backing_store), m_bkt_size(backing_store.size()) {
        while (m_backing_store[m_current_bucket].empty()) {
            m_current_bucket++;
            if (m_current_bucket == m_backing_store.size()) {
                m_current_bucket       = npos;
                m_current_vector_index = npos;
                break;
            }
        }
    }
    HashTableIterator(HashTableStorage<T>& backing_store, size_t bucket_index, size_t vector_index) :
        m_backing_store(backing_store), m_current_bucket(bucket_index), m_current_vector_index(vector_index),
        m_bkt_size(backing_store.size()) {}
    HashTableIterator(HashTableIterator<T>&& other) noexcept            = default;
    HashTableIterator(const HashTableIterator<T>& other)                = default;
    HashTableIterator& operator=(HashTableIterator<T>&& other) noexcept = default;
    HashTableIterator& operator=(const HashTableIterator<T>& other)     = default;
    bool operator==(const HashTableIterator<T>& other) const {
        return &m_backing_store == &other.m_backing_store && m_current_bucket == other.m_current_bucket &&
               m_current_vector_index == other.m_current_vector_index;
    }
    bool operator!=(const HashTableIterator<T>& other) const {
        return &m_backing_store != &other.m_backing_store || m_current_bucket != other.m_current_bucket ||
               m_current_vector_index != other.m_current_vector_index;
    }
    T& operator*() { return m_backing_store[m_current_bucket][m_current_vector_index]; }
    HashTableIterator& operator++() {
        if (m_backing_store[m_current_bucket].empty() || m_current_vector_index >= m_backing_store[m_current_bucket].size() - 1) {
            if (++m_current_bucket == m_bkt_size) {
                // we're at end
                m_current_vector_index = npos;
                m_current_bucket       = npos;
                return *this;
            }
            while (m_backing_store[m_current_bucket++].empty()) {
                if (m_current_bucket == m_bkt_size) {
                    // we're at end
                    m_current_vector_index = npos;
                    m_current_bucket       = npos;
                    return *this;
                }
            }
            m_current_bucket--;
            m_current_vector_index = 0;
        } else {
            m_current_vector_index++;
        }
        return *this;
    }
    HashTableIterator operator++(int) {
        auto copy = *this;
        this->operator++();
        return copy;
    }
    HashTableIterator& operator--() {
        if (m_backing_store[m_current_bucket].empty() || m_current_vector_index == 0) {
            if (m_current_bucket == 0 || m_backing_store[m_current_bucket - 1].empty()) {
                // end iterator has npos
                m_current_vector_index = npos;
                m_current_bucket       = npos;
                return *this;
            }
            m_current_vector_index = m_backing_store[m_current_bucket - 1].size() - 1;
            m_current_bucket--;
        } else {
            m_current_vector_index--;
        }
        return *this;
    }
    HashTableIterator operator--(int) {
        auto copy = *this;
        this->operator--();
        return copy;
    }
};
template <typename T>
class ConstHashTableIterator {
    const HashTableStorage<T>& m_backing_store;
    size_t m_current_bucket       = 0;
    size_t m_current_vector_index = 0;
    size_t m_bkt_size             = 0;

    public:
    constexpr static size_t npos = static_cast<size_t>(-1);
    explicit ConstHashTableIterator(const HashTableStorage<T>& backing_store) :
        m_backing_store(backing_store), m_bkt_size(backing_store.size()) {
        while (m_backing_store[m_current_bucket].empty()) {
            m_current_bucket++;
            if (m_current_bucket == m_backing_store.size()) {
                m_current_bucket       = npos;
                m_current_vector_index = npos;
                break;
            }
        }
    }
    ConstHashTableIterator(const HashTableStorage<T>& backing_store, size_t bucket_index, size_t vector_index) :
        m_backing_store(backing_store), m_current_bucket(bucket_index), m_current_vector_index(vector_index),
        m_bkt_size(backing_store.size()) {}
    ConstHashTableIterator(ConstHashTableIterator<T>&& other) noexcept            = default;
    ConstHashTableIterator(const ConstHashTableIterator<T>& other)                = default;
    ConstHashTableIterator& operator=(ConstHashTableIterator<T>&& other) noexcept = default;
    ConstHashTableIterator& operator=(const ConstHashTableIterator<T>& other)     = default;
    bool operator==(const ConstHashTableIterator<T>& other) const {
        return &m_backing_store == &other.m_backing_store && m_current_bucket == other.m_current_bucket &&
               m_current_vector_index == other.m_current_vector_index;
    }
    bool operator!=(const ConstHashTableIterator<T>& other) const {
        return &m_backing_store != &other.m_backing_store || m_current_bucket != other.m_current_bucket ||
               m_current_vector_index != other.m_current_vector_index;
    }
    const T& operator*() { return m_backing_store[m_current_bucket][m_current_vector_index]; }
    ConstHashTableIterator& operator++() {
        if (m_backing_store[m_current_bucket].empty() || m_current_vector_index >= m_backing_store[m_current_bucket].size() - 1) {
            if (++m_current_bucket == m_bkt_size) {
                // we're at end
                m_current_vector_index = npos;
                m_current_bucket       = npos;
                return *this;
            }
            while (m_backing_store[m_current_bucket++].empty()) {
                if (m_current_bucket == m_bkt_size) {
                    // we're at end
                    m_current_vector_index = npos;
                    m_current_bucket       = npos;
                    return *this;
                }
            }
            m_current_bucket--;
            m_current_vector_index = 0;
        } else {
            m_current_vector_index++;
        }
        return *this;
    }
    ConstHashTableIterator operator++(int) {
        auto copy = *this;
        this->operator++();
        return copy;
    }
    ConstHashTableIterator& operator--() {
        if (m_backing_store[m_current_bucket].empty() || m_current_vector_index == 0) {
            if (m_current_bucket == 0 || m_backing_store[m_current_bucket - 1].empty()) {
                // end iterator has npos
                m_current_vector_index = npos;
                m_current_bucket       = npos;
                return *this;
            }
            m_current_vector_index = m_backing_store[m_current_bucket - 1].size() - 1;
            m_current_bucket--;
        } else {
            m_current_vector_index--;
        }
        return *this;
    }
    ConstHashTableIterator operator--(int) {
        if (m_backing_store[m_current_bucket].empty() || m_current_vector_index == 0) {
            return { m_backing_store, m_current_bucket == 0 ? npos : m_current_bucket - 1,
                     m_current_bucket == 0 ? npos : m_backing_store[m_current_bucket - 1].size() - 1 };
        } else {
            return { m_backing_store, m_current_bucket, m_current_vector_index - 1 };
        }
    }
};
template <typename T, typename HashCls = Hash<T>>
requires Hashable<T, HashCls> && EqualityComparable<T> && requires(const HashCls& h, const T& t) {
                                                              { h(t) } -> ConvertibleTo<size_t>;
                                                          }
class HashTable {
    constexpr static inline size_t s_primes_bkt_sizes[]       = { 61, 97, 149, 223, 257, 281, 317, 379, 433, 503 };
    constexpr static inline size_t max_bucket_acceptable_size = 10;
    HashTableStorage<T> m_storage;
    size_t m_bucket_count = s_primes_bkt_sizes[0];
    size_t m_size         = 0;
    size_t m_max_bkt_size = 0;
    uint8_t m_curr_bkts   = 0;
    HashCls m_hasher{};

    using Iter      = HashTableIterator<T>;
    using ConstIter = ConstHashTableIterator<T>;
    template <typename... Args>
    void internal_append(T&& arg, Args&&... args) {
        insert(Forward<T>(arg));
        if constexpr (sizeof...(args) > 0) internal_append(Forward<Args>(args)...);
    }
    void rehash_internal_() {
        m_max_bkt_size = 0;
        if (m_curr_bkts == sizeof_array(s_primes_bkt_sizes) - 1) return;
        m_bucket_count   = s_primes_bkt_sizes[++m_curr_bkts];
        auto new_storage = HashTableStorage<T>{};
        new_storage.resize(m_bucket_count);
        for (auto& vec : m_storage) {
            for (auto&& item : vec) {
                auto hs        = m_hasher(item);
                auto bkt_index = hs % m_bucket_count;
                new_storage[bkt_index].append(Forward<T>(item));
                auto bkt_size = new_storage[bkt_index].size();
                if (bkt_size > m_max_bkt_size) m_max_bkt_size = bkt_size;
            }
        }
        m_storage.clear();
        m_storage = ARLib::move(new_storage);
    }
    public:
    HashTable() { m_storage.resize(m_bucket_count); }
    HashTable(const HashTable& other) :
        m_storage(other.m_storage), m_bucket_count(other.m_bucket_count), m_size(other.m_size), m_hasher(other.hasher) {
    }
    HashTable(HashTable&& other) noexcept :
        m_storage(move(other.m_storage)), m_bucket_count(other.m_bucket_count), m_size(other.m_size),
        m_hasher(move(other.m_hasher)) {}
    explicit HashTable(size_t initial_bucket_count) : m_size(initial_bucket_count) {
        m_storage.resize(initial_bucket_count);
    }
    HashTable& operator=(const HashTable& other) {
        m_storage      = other.m_storage;
        m_bucket_count = other.m_bucket_count;
        m_size         = other.m_size;
        m_hasher       = other.m_hasher;
        return *this;
    }
    HashTable& operator=(HashTable&& other) noexcept {
        m_storage      = move(other.m_storage);
        m_bucket_count = other.m_bucket_count;
        m_size         = other.m_size;
        m_hasher       = other.m_hasher;
        return *this;
    }
    template <typename... Args>
    HashTable(T&& a, T&& b, Args&&... args) {
        m_storage.resize(m_bucket_count);
        insert(Forward<T>(a));
        insert(Forward<T>(b));
        if constexpr (sizeof...(args) > 0) internal_append(Forward<Args>(args)...);
    }
    size_t max_bucket_size() { return m_max_bkt_size; }
    double load() {
        if (m_size == 0) return 0.0;
        double avg_load = static_cast<double>(m_size) / static_cast<double>(m_bucket_count);
        double sr       = sum(m_storage, [avg_load](const auto& item) {
            double ll = static_cast<double>(item.size()) - avg_load;
            return ll * ll;
        });
        double res      = sqrt(sr / static_cast<double>(m_size));
        return res;
    }
    void clear() {
        m_storage.clear_retain();
        m_bucket_count = s_primes_bkt_sizes[0];
        m_storage.resize(m_bucket_count);
        m_size         = 0;
        m_max_bkt_size = 0;
        m_curr_bkts    = 0;
    }
    template <typename Functor>
    void for_each(Functor func) {
        m_storage.for_each([&func](auto& bkt) { bkt.for_each(func); });
    }
    void for_each(void (*func)(const T&)) const {
        m_storage.for_each([&func](const auto& bkt) { bkt.for_each(func); });
    }
    InsertionResult insert(const T& val) {
        T entry = val;
        return insert(Forward<T>(entry));
    }
    InsertionResult insert(T&& entry) {
        if (max_bucket_size() >= max_bucket_acceptable_size) { rehash_internal_(); }
        auto hs   = m_hasher(entry);
        auto iter = find(entry);
        if (iter == tend()) {
            auto bkt_index = hs % m_bucket_count;
            m_storage[bkt_index].append(Forward<T>(entry));
            auto bkt_size = m_storage[bkt_index].size();
            if (bkt_size > m_max_bkt_size) m_max_bkt_size = bkt_size;
            m_size++;
            return InsertionResult::New;
        } else {
            (*iter) = ARLib::move(entry);
            return InsertionResult::Replace;
        }
    }
    HashTableIterator<T> find(const T& val) {
        auto hash           = m_hasher(val);
        size_t bucket_index = hash % m_bucket_count;
        const auto& bucket  = m_storage[bucket_index];
        for (size_t i = 0; i < bucket.size(); i++) {
            auto& item = bucket[i];
            if (hash == m_hasher(item) && item == val) return { m_storage, bucket_index, i };
        }
        return tend();
    }
    ConstHashTableIterator<T> find(const T& val) const {
        auto hash           = m_hasher(val);
        size_t bucket_index = hash % m_bucket_count;
        const auto& bucket  = m_storage[bucket_index];
        for (size_t i = 0; i < bucket.size(); i++) {
            auto& item = bucket[i];
            if (hash == m_hasher(item))
                if (item == val) return { m_storage, bucket_index, i };
        }
        return tend();
    }
    template <typename Functor>
    HashTableIterator<T> find_if(size_t hash, Functor func) {
        size_t bucket_index = hash % m_bucket_count;
        const auto& bucket  = m_storage[bucket_index];
        for (size_t i = 0; i < bucket.size(); i++) {
            auto& item = bucket[i];
            if (hash == m_hasher(item))
                if (func(item)) return { m_storage, bucket_index, i };
        }
        return tend();
    }
    template <typename Functor>
    ConstHashTableIterator<T> find_if(size_t hash, Functor func) const {
        size_t bucket_index = hash % m_bucket_count;
        const auto& bucket  = m_storage[bucket_index];
        for (size_t i = 0; i < bucket.size(); i++) {
            auto& item = bucket[i];
            if (hash == m_hasher(item))
                if (func(item)) return { m_storage, bucket_index, i };
        }
        return tend();
    }
    auto end(size_t hash) { return m_storage[hash % m_bucket_count].end(); }
    HashTableIterator<T> begin() { return HashTableIterator<T>{ m_storage }; }
    HashTableIterator<T> end() {
        return HashTableIterator<T>{ m_storage, HashTableIterator<T>::npos, HashTableIterator<T>::npos };
    }
    ConstHashTableIterator<T> begin() const { return ConstHashTableIterator<T>{ m_storage }; }
    ConstHashTableIterator<T> end() const {
        return ConstHashTableIterator<T>{ m_storage, HashTableIterator<T>::npos, HashTableIterator<T>::npos };
    }
    HashTableIterator<T> tbegin() { return HashTableIterator<T>{ m_storage }; }
    HashTableIterator<T> tend() {
        return HashTableIterator<T>{ m_storage, HashTableIterator<T>::npos, HashTableIterator<T>::npos };
    }
    ConstHashTableIterator<T> tbegin() const { return ConstHashTableIterator<T>{ m_storage }; }
    ConstHashTableIterator<T> tend() const {
        return ConstHashTableIterator<T>{ m_storage, HashTableIterator<T>::npos, HashTableIterator<T>::npos };
    }
    DeletionResult remove(const T& val) {
        auto hs   = m_hasher(val);
        auto iter = find(val);
        if (iter == tend()) { return DeletionResult::Failure; }
        m_storage[hs % m_bucket_count].remove(val);
        m_size--;
        return DeletionResult::Success;
    }
    size_t size() const { return m_size; }
    size_t bucket_count() const { return m_bucket_count; }
};
}    // namespace ARLib
