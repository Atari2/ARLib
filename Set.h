#pragma once
#include <initializer_list>
#include "Concepts.h"
#include "Assertion.h"
#include "Iterator.h"

namespace ARLib {
	template <EqualityComparable T>
	class Set {
		using Iter = Iterator<T>;
		using ReverseIter = ReverseIterator<T>;
		using ConstIter = Iterator<T>;
		using ConstReverseIter = ReverseIterator<T>;

		size_t m_capacity = 0;
		size_t m_size = 0;
		T* m_storage = nullptr;

		void grow_internal_(size_t capacity) {
			HARD_ASSERT_FMT((capacity > m_capacity && capacity > m_size), "You can't grow by shrinking the size, size = %lu, attempted capacity = %lu", m_size, capacity)
			m_capacity = capacity;
			T* new_storage = new T[m_capacity];
			for (size_t i = 0; i < m_size; i++)
				new_storage[i] = move(m_storage[i]);
			delete[] m_storage;
			m_storage = new_storage;
		}

		void check_capacity_() {
			if (m_size == m_capacity) grow_internal_(m_capacity == 0 ? 1 : m_capacity * 2);
		}

		void assert_index_(size_t index) {
			SOFT_ASSERT_FMT((index < m_size), "Index %lu was higher than size %lu", index, m_size);
		}

		void append_internal_(T&& elem) {
			check_capacity_();
			m_storage[m_size++] = move(elem);
		}
		void append_internal_(const T& elem) {
			check_capacity_();
			m_storage[m_size++] = elem;
		}

		void clear_() {
			if (m_capacity == 0) return;
			for (size_t i = 0; i < m_size; i++)
				m_storage[i].~T();
			delete[] m_storage;
			m_storage = nullptr;
		}

	public:
		Set() = default;
		Set(size_t capacity) : m_capacity(capacity), m_storage(new T[capacity]) {

		}
		Set(std::initializer_list<T> list) {
			grow_internal_(list.size());
			for (auto& item : list)
				m_storage[m_size++] = item;
		}

		bool insert(const T& elem) requires CopyAssignable<T> {
			Iter it = find(elem);
			if (it != end()) { return false; }
			else { append_internal_(elem); }
			return true;
		}

		bool insert(T&& elem) requires MoveAssignable<T> {
			Iter it = find(elem);
			if (it != end()) { return false; }
			else { append_internal_(Forward<T>(elem)); }
			return true;
		}

		Iter begin() const {
			return { m_storage };
		}

		Iter end() const {
			return { m_storage + m_size };
		}

		ConstIter cbegin() const {
			return { m_storage };
		}

		ConstIter cend() const {
			return { m_storage + m_size };
		}

		ReverseIter rbegin() const {
			return { m_storage + m_size - 1 };
		}

		ReverseIter rend() const {
			return { m_storage - 1 };
		}

		ConstReverseIter crbegin() const {
			return { m_storage + m_size - 1 };
		}

		ConstReverseIter crend() const {
			return { m_storage - 1 };
		}

		bool remove(size_t index) {
			if (index >= m_size) return false;
			return remove(m_storage[index]);
		}

		bool remove(const T& elem) {
			for (size_t i = 0; i < m_size; i++) {
				if (m_storage[i] == elem) {
					m_storage[i].~T();
					m_size--;
					memmove(m_storage + i, m_storage + i + 1, sizeof(T) * (m_size - i));
					return true;
				}
			}
			return false;
		}

		Iter find(const T& elem) const {
			for (size_t i = 0; i < m_size; i++) {
				if (m_storage[i] == elem) return { m_storage + i };
			}
			return end();
		}

		bool contains(const T& elem) const {
			return find(elem) != end();
		}

		size_t size() const { return m_size; }
		size_t capacity() const { return m_capacity; }

		void reserve(size_t capacity) { grow_internal_(capacity); }
		T& operator[](size_t index) { assert_index_(index); return m_storage[index]; }
		const T& operator[](size_t index) const { assert_index_(index); return m_storage[index]; }

		template<typename Functor>
		void for_each(Functor&& func) {
			for (size_t i = 0; i < m_size; i++)
				func(m_storage[i]);
		}

		void clear() {
			clear_();
		}

		~Set() {
			if (m_capacity == 0) return;
			for (size_t i = 0; i < m_size; i++)
				m_storage[i].~T();
			delete[] m_storage;
			m_storage = nullptr;
		}

	};
}

using ARLib::Set;