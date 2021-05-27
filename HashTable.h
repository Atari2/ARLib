#pragma once
#include "Concepts.h"
#include "Assertion.h"
#include "HashCalc.h"
#include "Algorithm.h"
#include "List.h"
#include "Vector.h"

namespace ARLib {
	template <typename T, size_t TBL_SIZE = 0> requires Hashable<T> && EqualityComparable<T>
	class HashTable {
		static constexpr size_t table_sizes[] = { 13, 19, 31 };
		Vector<LinkedList<T>> m_storage;
		size_t m_size = table_sizes[TBL_SIZE];
		
		template <typename... Args>
		void internal_append(T&& arg, Args&&... args) {
			insert(Forward<T>(arg));
			if constexpr (sizeof...(args) > 0)
				internal_append(Forward<Args>(args)...);
		}

	public:
		
		HashTable() {
			m_storage.resize(m_size);
		}
		HashTable(size_t initial_capacity) : m_size(initial_capacity) {
			m_storage.resize(initial_capacity);
		}
		template<typename... Args>
		HashTable(T&& a, T&& b, Args&&... args) {
			m_storage.resize(m_size);
			insert(Forward<T>(a));
			insert(Forward<T>(b));
			if constexpr (sizeof...(args) > 0)
				internal_append(Forward<Args>(args)...);
		}

		float load() {
			size_t s = sum(m_storage, [](const auto& item) {
				return item.size();
			});
			float avg_load = (float)s / (float)m_size;
			return avg_load;
		}

		template <typename Functor>
		void for_each(Functor&& func) {
			int i = 0;
			m_storage.for_each([&func, &i](auto& bkt) {
				printf("Bucket %d (size of bucket %llu):\n", i++, bkt.size());
				bkt.for_each(func);
			});
		}

		void insert(const T& val) {
			T v = val;
			m_storage[hash(val) % m_size].prepend(Forward<T>(v));
		}
		void insert(T&& val) {
			m_storage[hash(val) % m_size].prepend(Forward<T>(val));
		}
		
		const T& find(const T& val) {
			return *m_storage[hash(val) % m_size].find(val);
		}

		void remove(const T& val) {
			m_storage[hash(val) % m_size].remove(val);
		}

	};
}

using ARLib::HashTable;