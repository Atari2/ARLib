#pragma once
#include "Concepts.h"
#include "Assertion.h"
#include "HashBase.h"
#include "Algorithm.h"
#include "List.h"
#include "Vector.h"
#include "cmath_compat.h"

namespace ARLib {
	enum class InsertionResult {
		New,
		Replace
	};

	template <typename T, size_t BKT_SIZE>
	class HashTableIterator {
		Vector<Vector<T>>& m_backing_store;
		size_t m_current_bucket = 0;
		size_t m_current_vector_index = 0;
	public:
		static constexpr size_t npos = static_cast<size_t>(-1);
		HashTableIterator(Vector<Vector<T>>& backing_store) : m_backing_store(backing_store) {

		}

		HashTableIterator(Vector<Vector<T>>& backing_store, size_t bucket_index, size_t vector_index) 
			: m_backing_store(backing_store), m_current_bucket(bucket_index), m_current_vector_index(vector_index) {

		}

		HashTableIterator(HashTableIterator<T, BKT_SIZE>&& other) = default;
		HashTableIterator(const HashTableIterator<T, BKT_SIZE>& other) = default;
		HashTableIterator& operator=(HashTableIterator<T, BKT_SIZE>&& other) = default;
		HashTableIterator& operator=(const HashTableIterator<T, BKT_SIZE>& other) = default;

		bool operator==(const HashTableIterator<T, BKT_SIZE>& other) const {
			return &m_backing_store == &other.m_backing_store && m_current_bucket == other.m_current_bucket && m_current_vector_index == other.m_current_vector_index;
		}

		bool operator!=(const HashTableIterator<T, BKT_SIZE>& other) const {
			return &m_backing_store != &other.m_backing_store || m_current_bucket != other.m_current_bucket || m_current_vector_index != other.m_current_vector_index;
		}

		T& operator*() {
			return m_backing_store[m_current_bucket][m_current_vector_index];
		}

		HashTableIterator& operator++() {
			if (m_backing_store[m_current_bucket].empty() || m_current_vector_index >= m_backing_store[m_current_bucket].size() - 1) {
				if (m_current_bucket == BKT_SIZE - 1 || m_backing_store[m_current_bucket + 1].empty()) {
					// end iterator has npos
					m_current_vector_index = npos;
					m_current_bucket = npos;
					return *this;
				}
				m_current_vector_index = 0;
				m_current_bucket++;
			}
			else {
				m_current_vector_index++;
			}
			return *this;
		}

		HashTableIterator operator++(int) {
			if (m_backing_store[m_current_bucket].empty() ||  m_current_vector_index >= m_backing_store[m_current_bucket].size() - 1) {
				return { m_backing_store, m_current_bucket == BKT_SIZE - 1 ? npos : m_current_bucket + 1, m_current_bucket == BKT_SIZE - 1 ? npos : 0};
			}
			else {
				return { m_backing_store, m_current_bucket, m_current_vector_index + 1 };
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
			}
			else {
				m_current_vector_index--;
			}
			return *this;
		}

		HashTableIterator operator--(int) {
			if (m_backing_store[m_current_bucket].empty() || m_current_vector_index == 0) {
				return { m_backing_store, m_current_bucket == 0 ? npos : m_current_bucket - 1, m_current_bucket == 0 ? npos : m_backing_store[m_current_bucket - 1].size() - 1 };
			}
			else {
				return { m_backing_store, m_current_bucket, m_current_vector_index - 1 };
			}
		}

	};

	template <typename T, size_t TBL_SIZE_INDEX = 0> requires Hashable<T> && EqualityComparable<T>
	class HashTable {
		Vector<Vector<T>> m_storage;
		size_t m_bucket_count = table_sizes[TBL_SIZE_INDEX];
		size_t m_size = 0;

		template <typename... Args>
		void internal_append(T&& arg, Args&&... args) {
			insert(Forward<T>(arg));
			if constexpr (sizeof...(args) > 0)
				internal_append(Forward<Args>(args)...);
		}

	public:
		static constexpr size_t table_sizes[3] = { 13, 19, 31 };
		Hash<T> hasher{};
		HashTable() {
			m_storage.resize(m_bucket_count);
		}
		HashTable(size_t initial_bucket_count) : m_size(initial_bucket_count) {
			m_storage.resize(initial_bucket_count);
		}
		template<typename... Args>
		HashTable(T&& a, T&& b, Args&&... args) {
			m_storage.resize(m_bucket_count);
			insert(Forward<T>(a));
			insert(Forward<T>(b));
			if constexpr (sizeof...(args) > 0)
				internal_append(Forward<Args>(args)...);
			m_size = sizeof...(args);
		}

		double load() {
			Vector<double> vec_sizes(m_storage.size());
			double avg_load = (double)m_size / (double)m_bucket_count;
			double sr = sum(vec_sizes, [avg_load](const auto& item) {
				return pow(item - avg_load, 2);
			});
			double res = sqrt(sr / (double)m_size);
			return res;
		}

		template <typename Functor>
		void for_each(Functor func) {
			int i = 0;
			m_storage.for_each([&func](auto& bkt) {
				bkt.for_each(func);
			});
		}

		InsertionResult insert(const T& val) {
			T entry = val;
			return insert(Forward<T>(entry));
		}

		InsertionResult insert(T&& entry) {
			auto hs = hasher(entry);
			auto iter = find(entry);
			if (iter == end(hs)) {
				m_storage[hs % m_bucket_count].append(Forward<T>(entry));
				m_size++;
				return InsertionResult::New;
			}
			else {
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
					if (item.key() == val.key())
						return it;
			}
			return bucket.end();
		}

		template <typename Functor>
		auto find_if(size_t hash, Functor func) {
			const auto& bucket = m_storage[hash % m_bucket_count];
			for (auto it = bucket.begin(); it != bucket.end(); it++) {
				auto& item = *it;
				if (hash == hasher(*it))
					if (func(*it))
						return it;
			}
			return bucket.end();
		}

		const auto end(size_t hash) {
			return m_storage[hash % m_bucket_count].end();
		}

		HashTableIterator<T, table_sizes[TBL_SIZE_INDEX]> tbegin() {
			return {m_storage, 0, 0};
		}

		HashTableIterator<T, table_sizes[TBL_SIZE_INDEX]> tend() {
			return { m_storage, HashTableIterator<T, table_sizes[TBL_SIZE_INDEX]>::npos, HashTableIterator < T, table_sizes[TBL_SIZE_INDEX]>::npos };
		}
		
		void remove(const T& val) {
			m_storage[hash(val) % m_bucket_count].remove(val);
		}

		size_t size() { return m_size; }
		size_t bucket_count() { return m_bucket_count;  }

	};
}

using ARLib::HashTable;
using ARLib::HashTableIterator;
using ARLib::InsertionResult;