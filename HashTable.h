#pragma once
#include "Concepts.h"
#include "Assertion.h"
#include "HashCalc.h"
#include "Algorithm.h"
#include "List.h"
#include "Vector.h"
#include "cmath_compat.h"

namespace ARLib {
	template <typename T, size_t TBL_SIZE = 0> requires Hashable<T> && EqualityComparable<T>
	class HashTable {
		// this is slow af
		static constexpr size_t table_sizes[] = { 13, 19, 31 };
		Vector<Vector<T>> m_storage;
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

		double load() {
			Vector<double> vec_sizes(m_storage.size());
			size_t s = sum(m_storage, [&vec_sizes](const auto& item) {
				vec_sizes.append(item.size());
				return item.size();
			});
			double avg_load = (double)s / (double)m_size;
			double sr = sum(vec_sizes, [avg_load](const auto& item) {
				return pow(item - avg_load, 2);
			});
			double res = sqrt(sr / (double)m_size);
			return res;
		}

		template <typename Functor>
		void for_each(Functor&& func) {
			int i = 0;
			m_storage.for_each([&func, &i](auto& bkt) {
				bkt.for_each(func);
			});
		}

		void insert(const T& val) {
			T v = val;
			m_storage[hash(val) % m_size].append(Forward<T>(v));
		}
		void insert(T&& val) {
			m_storage[hash(val) % m_size].append(Forward<T>(val));
		}

		void insert_precalc(const T& val, uint32_t hash) {
			T v = val;
			m_storage[hash % m_size].append(Forward<T>(v));
		}
		void insert_precalc(T&& val, uint32_t hash) {
			m_storage[hash % m_size].append(Forward<T>(val));
		}


		const auto find_uncertain(const T& val) {
			return m_storage[hash(val) % m_size].find(val);
		}

		const auto find_uncertain_precalc(const T& val, uint32_t hash) {
			return m_storage[hash % m_size].find(val);
		}

		const auto end(uint32_t hash) {
			return m_storage[hash % m_size].end();
		}

		const auto end_precalc(uint32_t hash) {
			return m_storage[hash % m_size].end();
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