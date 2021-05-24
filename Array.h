#pragma once

#include "Utility.h"
#include "Assertion.h"

namespace ARLib {
	template <typename T, size_t S>
	class Array {
		T m_storage[S];
		const size_t m_capacity = S;
		size_t m_size = 0;

		void assert_size_(size_t index) {
			SOFT_ASSERT_FMT((index < m_size), "Index %lu out of bounds in array of size %lu", index, m_size)
		}

		void assert_capacity_() {
			SOFT_ASSERT_FMT((m_size < m_capacity), "Size %lu can't be higher than capacity %lu", m_size, m_capacity)
		}

		template <typename... Args>
		void append_internal_(T&& value, Args&&... args) {
			m_storage[m_size++] = move(value);
			append_internal_(Forward<Args>(args)...);
		}

		void append_internal_(T&& value) {
			m_storage[m_size++] = move(value);
		}

	public:
		Array() = default;

		template <typename... Args>
		Array(Args&&... args) {
			HARD_ASSERT_FMT((sizeof...(args) <= S), "More arguments %lu than size of array %lu", sizeof...(args), S);
			append_internal_(Forward<Args>(args)...);
		}

		constexpr size_t capacity() const { return m_capacity; }
		size_t size() const { return m_size; }

		T& index(size_t index) {
			assert_size_(index);
			return m_storage[index];
		}

		const T& index(size_t index) const {
			assert_size_(index);
			return m_storage[index];
		}

		T& operator[](size_t index) {
			assert_size_(index);
			return m_storage[index];
		}

		const T& operator[](size_t index) const {
			assert_size_(index);
			return m_storage[index];
		}

		void append(T&& value) {
			assert_capacity_();
			m_storage[m_size++] = move(value);
		}


		void append(const T& value) {
			assert_capacity_();
			m_storage[m_size++] = value;
		}

		template <typename... Args>
		void emplace(Args... args) {
			assert_capacity_();
			m_storage[m_size++] = T{ args... };
		}

		void clear() {
			for (size_t i = 0; i < m_size; i++)
				m_storage[i].~T();
			m_size = 0;
		}

		~Array() {
			clear();
		}

	};
}

using ARLib::Array;