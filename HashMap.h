#pragma once
#include "HashBase.h"
#include "Concepts.h"
#include "Assertion.h"
#include "HashTable.h"

namespace ARLib {

	enum class InsertionResult {
		New,
		Replace
	};

	template <Hashable Key, EqualityComparable Val>
	class HashMap {
		struct Entry {
			Key m_key;
			Val m_value;
		public:
			Entry(Key key, Val value) : m_key(move(key)), m_value(move(value)) {

			}
			Entry(Entry&& other) noexcept : m_key(move(other.m_key)), m_value(move(other.m_value)) {

			}
			Entry(const Entry& other) : m_key(other.m_key), m_value(other.m_value) {

			}

			Entry& operator=(Entry&& other) noexcept {
				m_key = move(other.m_key);
				m_value = move(other.m_value);
				return *this;
			}
			bool operator==(const Entry& other) {
				return hash_equals(*this, other);
			}

			bool operator!=(const Entry& other) {
				return !hash_equals(*this, other);
			}

			friend uint32_t hash(const Entry& s) {
				return hash(s.m_key);
			}
			friend bool hash_equals(const Entry& a, const Entry& b) {
				return hash(a) == hash(b);
			}
		};

		size_t m_size = 0;

		HashTable<Entry> m_table{};
	public:
		HashMap() = default;

		InsertionResult add(Key key, Val value) {
			Entry entry{ key, value };
			uint32_t hs = hash(entry);
			auto iter = m_table.find_uncertain_precalc(entry, hs);
			if (iter == m_table.end_precalc(hs)) {
				m_table.insert(Forward<Entry>(entry));
				m_size++;
				return InsertionResult::New;
			}
			else {
				(*iter) = move(entry);
				return InsertionResult::Replace;
			}
		}
		InsertionResult add(Entry entry) {
			uint32_t hs = hash(entry);
			auto iter = m_table.find_uncertain_precalc(entry, hs);
			if (iter == m_table.end_precalc(hs)) {
				m_table.insert(Forward<Entry>(entry));
				m_size++;
				return InsertionResult::New;
			}
			else {
				(*iter) = move(entry);
				return InsertionResult::Replace;
			}
		}

		size_t size() const { return m_size; }
	};
}

using ARLib::HashMap;
using ARLib::InsertionResult;