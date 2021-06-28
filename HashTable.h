#pragma once
#include "Algorithm.h"
#include "Assertion.h"
#include "Concepts.h"
#include "HashBase.h"
#include "List.h"
#include "Vector.h"
#include "cmath_compat.h"

namespace ARLib {
    enum class InsertionResult { New, Replace };

    template <typename T>
    class HashTableIterator {
        Vector<Vector<T>>& m_backing_store;
        size_t m_current_bucket = 0;
        size_t m_current_vector_index = 0;
        size_t m_bkt_size = 0;

        public:
        static constexpr size_t npos = static_cast<size_t>(-1);
        HashTableIterator(Vector<Vector<T>>& backing_store) :
            m_backing_store(backing_store), m_bkt_size(backing_store.size()) {
            while (m_backing_store[m_current_bucket].empty()) {
                m_current_bucket++;
            }
        }

        HashTableIterator(Vector<Vector<T>>& backing_store, size_t bucket_index, size_t vector_index) :
            m_backing_store(backing_store),
            m_current_bucket(bucket_index),
            m_current_vector_index(vector_index),
            m_bkt_size(backing_store.size()) {}

        HashTableIterator(HashTableIterator<T>&& other) = default;
        HashTableIterator(const HashTableIterator<T>& other) = default;
        HashTableIterator& operator=(HashTableIterator<T>&& other) = default;
        HashTableIterator& operator=(const HashTableIterator<T>& other) = default;

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
            if (m_backing_store[m_current_bucket].empty() ||
                m_current_vector_index >= m_backing_store[m_current_bucket].size() - 1) {
                if (m_current_bucket == m_bkt_size - 1 || m_backing_store[m_current_bucket + 1].empty()) {
                    // end iterator has npos
                    m_current_vector_index = npos;
                    m_current_bucket = npos;
                    return *this;
                }
                m_current_vector_index = 0;
                m_current_bucket++;
            } else {
                m_current_vector_index++;
            }
            return *this;
        }

        HashTableIterator operator++(int) {
            if (m_backing_store[m_current_bucket].empty() ||
                m_current_vector_index >= m_backing_store[m_current_bucket].size() - 1) {
                return {m_backing_store, m_current_bucket == m_bkt_size - 1 ? npos : m_current_bucket + 1,
                        m_current_bucket == m_bkt_size - 1 ? npos : 0};
            } else {
                return {m_backing_store, m_current_bucket, m_current_vector_index + 1};
            }
        }

        HashTableIterator& operator--() {
            if (m_backing_store[m_current_bucket].empty() || m_current_vector_index == 0) {
                if (m_current_bucket == 0 || m_backing_store[m_current_bucket - 1].empty()) {
                    // end iterator has npos
                    m_current_vector_index = npos;
                    m_current_bucket = npos;
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
            if (m_backing_store[m_current_bucket].empty() || m_current_vector_index == 0) {
                return {m_backing_store, m_current_bucket == 0 ? npos : m_current_bucket - 1,
                        m_current_bucket == 0 ? npos : m_backing_store[m_current_bucket - 1].size() - 1};
            } else {
                return {m_backing_store, m_current_bucket, m_current_vector_index - 1};
            }
        }
    };

    template <typename T, size_t TBL_SIZE_INDEX = 0>
    requires Hashable<T>&& EqualityComparable<T> class HashTable {
        static constexpr inline double load_factor = 3.0;
        Vector<Vector<T>> m_storage;
        size_t m_bucket_count = table_sizes[TBL_SIZE_INDEX];
        size_t m_size = 0;

        template <typename... Args>
        void internal_append(T&& arg, Args&&... args) {
            insert(Forward<T>(arg));
            if constexpr (sizeof...(args) > 0) internal_append(Forward<Args>(args)...);
        }

        void rehash_internal_() {
            m_bucket_count = prime_generator(m_bucket_count);
            auto new_storage = Vector<Vector<T>>{};
            new_storage.resize(m_bucket_count);
            for (auto& vec : m_storage) {
                for (auto& item : vec) {
                    auto hs = hasher(item);
                    new_storage[hs % m_bucket_count].append(Forward<T>(item));
                }
            }
            m_storage.clear();
            m_storage = move(new_storage);
        }

        public:
        static constexpr size_t table_sizes[3] = {13, 19, 31};
        Hash<T> hasher{};
        HashTable() { m_storage.resize(m_bucket_count); }
        HashTable(const HashTable& other) {
            m_storage = other.m_storage;
            m_bucket_count = other.m_bucket_count;
            m_size = other.m_size;
            hasher = other.hasher;
        }
        HashTable(size_t initial_bucket_count) : m_size(initial_bucket_count) {
            m_storage.resize(initial_bucket_count);
        }
        HashTable& operator=(const HashTable& other) {
            m_storage = other.m_storage;
            m_bucket_count = other.m_bucket_count;
            m_size = other.m_size;
            hasher = other.hasher;
            return *this;
        }
        template <typename... Args>
        HashTable(T&& a, T&& b, Args&&... args) {
            m_storage.resize(m_bucket_count);
            insert(Forward<T>(a));
            insert(Forward<T>(b));
            if constexpr (sizeof...(args) > 0) internal_append(Forward<Args>(args)...);
            m_size = sizeof...(args);
        }

        double load() {
            double avg_load = (double)m_size / (double)m_bucket_count;
            double sr = sum(m_storage, [avg_load](const auto& item) { return pow(item.size() - avg_load, 2); });
            double res = sqrt(sr / (double)m_size);
            return res;
        }

        template <typename Functor>
        void for_each(Functor func) {
            int i = 0;
            m_storage.for_each([&func](auto& bkt) { bkt.for_each(func); });
        }

        InsertionResult insert(const T& val) {
            T entry = val;
            return insert(Forward<T>(entry));
        }

        InsertionResult insert(T&& entry) {
            // this is not good, calculating load() every time is costy
            double ld = load();
            if (ld >= load_factor) {
                rehash_internal_();
            }
            auto hs = hasher(entry);
            auto iter = find(entry);
            if (iter == end(hs)) {
                m_storage[hs % m_bucket_count].append(Forward<T>(entry));
                m_size++;
                return InsertionResult::New;
            } else {
                (*iter) = move(entry);
                return InsertionResult::Replace;
            }
        }

        auto find(const T& val) {
            auto hash = hasher(val);
            const auto& bucket = m_storage[hash % m_bucket_count];
            for (auto it = bucket.begin(); it != bucket.end(); it++) {
                auto& item = *it;
                if (hash == hasher(item))
                    if (item.key() == val.key()) return it;
            }
            return bucket.end();
        }

        template <typename Functor>
        auto find_if(size_t hash, Functor func) {
            const auto& bucket = m_storage[hash % m_bucket_count];
            for (auto it = bucket.begin(); it != bucket.end(); it++) {
                auto& item = *it;
                if (hash == hasher(*it))
                    if (func(*it)) return it;
            }
            return bucket.end();
        }

        const auto end(size_t hash) { return m_storage[hash % m_bucket_count].end(); }

        HashTableIterator<T> tbegin() { return {m_storage}; }

        HashTableIterator<T> tend() { return {m_storage, HashTableIterator<T>::npos, HashTableIterator<T>::npos}; }

        void remove(const T& val) { m_storage[hash(val) % m_bucket_count].remove(val); }

        size_t size() { return m_size; }
        size_t bucket_count() { return m_bucket_count; }
    };
} // namespace ARLib

using ARLib::HashTable;
using ARLib::HashTableIterator;
using ARLib::InsertionResult;