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
	struct HashMapEntry {
		Key m_key;
		Val m_value;
	public:
		Hash<Key> hasher{};
		HashMapEntry() : m_key(), m_value() {

		}
		HashMapEntry(Key key, Val value) : m_key(move(key)), m_value(move(value)) {

		}
		HashMapEntry(HashMapEntry&& other) noexcept : m_key(move(other.m_key)), m_value(move(other.m_value)) {

		}
		HashMapEntry(const HashMapEntry& other) : m_key(other.m_key), m_value(other.m_value) {

		}

		HashMapEntry& operator=(HashMapEntry&& other) noexcept {
			m_key = move(other.m_key);
			m_value = move(other.m_value);
			return *this;
		}

		const Key& key() const {
			return m_key;
		}

		bool operator==(const HashMapEntry& other) {
			return hasher(this->m_key) == hasher(other.m_key);
		}

		bool operator!=(const HashMapEntry& other) {
			return hasher(this->m_key) != hasher(other.m_key);
		}
	};

	template <typename Key, typename Value>
	struct Hash<HashMapEntry<Key, Value>> {
		[[nodiscard]] forceinline size_t operator()(const HashMapEntry<Key, Value>& key) const noexcept {
			return key.hasher(key.key());
		}
	};

	template <Hashable Key, EqualityComparable Val, size_t TBL_SIZE = 0>
	class HashMap {
		using MapEntry = HashMapEntry<Key, Val>;
		size_t m_size = 0;
		HashTable<MapEntry, TBL_SIZE> m_table{};
	public:
		HashMap() = default;
		double load() { return m_table.load(); };
		InsertionResult add(Key key, Val value) {
			MapEntry entry{ key, value };
			auto hs = m_table.hasher(entry);
			auto iter = m_table.find_uncertain_precalc(entry, hs);
			if (iter == m_table.end_precalc(hs)) {
				m_table.insert(Forward<MapEntry>(entry));
				m_size++;
				return InsertionResult::New;
			}
			else {
				(*iter) = move(entry);
				return InsertionResult::Replace;
			}
		}
		InsertionResult add(MapEntry entry) {
			auto hs = m_table.hasher(entry);
			auto iter = m_table.find_uncertain_precalc(entry, hs);
			if (iter == m_table.end_precalc(hs)) {
				m_table.insert(Forward<MapEntry>(entry));
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