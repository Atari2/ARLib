#pragma once
#include "HashBase.h"
#include "Concepts.h"
#include "Assertion.h"

namespace ARLib {
	template <Hashable Key, EqualityComparable Val>
	class HashMap {
		struct Entry {
			Key key;
			Val value;
		};
		size_t m_size = 0;
		size_t m_capacity = 0;

		void resize_to_capacity_(size_t capacity) {
			m_capacity = capacity;
		}
	public:
		TODO(HashMap)
		// HashMap() = default;
		/*
		HashMap(std::initializer_list<Entry> list) {
			resize_to_capacity_(list.size());
			for (auto& item : list)
				add(item.key, item.value);
		}
		*/

		void add(Key key, Val value) {

		}
		void add(Entry entry) {

		}

		size_t size() const { return m_size; }
		size_t capacity() const { return m_capacity; }
	};
}

using ARLib::HashMap;