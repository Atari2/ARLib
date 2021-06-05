#pragma once
#include "HashBase.h"
#include "Concepts.h"
#include "Assertion.h"
#include "HashTable.h"

namespace ARLib {

	template <Hashable Key, typename Val>
	struct HashMapEntry {
		Key m_key;
		Val m_value;
		size_t m_hashval;
	public:
		HashMapEntry() : m_key(), m_value(), m_hashval(static_cast<size_t>(-1)) {

		}
		HashMapEntry(Key key, Val value) : m_key(move(key)), m_value(move(value)) {
			m_hashval = Hash<Key>{}(m_key);
		}
		HashMapEntry(HashMapEntry&& other) noexcept : m_key(move(other.m_key)), m_value(move(other.m_value)) {
			m_hashval = other.m_hashval;
		}
		HashMapEntry(const HashMapEntry& other) : m_key(other.m_key), m_value(other.m_value) {
			m_hashval = other.m_hashval;
		}

		HashMapEntry& operator=(HashMapEntry&& other) noexcept {
			m_key = move(other.m_key);
			m_value = move(other.m_value);
			m_hashval = other.m_hashval;
			return *this;
		}

		constexpr size_t hashval() const {
			return m_hashval;
		}

		const Key& key() const {
			return m_key;
		}
		const Val& value() const {
			return m_value;
		}

		Val& value() {
			return m_value;
		}

		bool operator==(const HashMapEntry& other) {
			return m_hashval == other.m_hashval;
		}

		bool operator!=(const HashMapEntry& other) {
			return m_hashval != other.m_hashval;
		}
	};

	template <typename Key, typename Value>
	struct Hash<HashMapEntry<Key, Value>> {
		[[nodiscard]] forceinline size_t operator()(const HashMapEntry<Key, Value>& key) const noexcept {
			return key.hashval();
		}
	};

	template <Hashable Key, typename Val, size_t TBL_SIZE_INDEX = 0>
	class HashMap {
		using MapEntry = HashMapEntry<Key, Val>;
		using Iter = HashTableIterator<MapEntry, HashTable<MapEntry, TBL_SIZE_INDEX>::table_sizes[TBL_SIZE_INDEX]>;
		HashTable<MapEntry, TBL_SIZE_INDEX> m_table{};
	public:
		HashMap() = default;
		double load() { return m_table.load(); };

		InsertionResult add(Key key, Val value) {
			MapEntry entry{ key, value };
			return m_table.insert(Forward<MapEntry>(entry));
		}
		InsertionResult add(MapEntry entry) {
			return m_table.insert(Forward<MapEntry>(entry));
		}

		template <typename Functor>
		void for_each(Functor func) {
			m_table.for_each([&func](const MapEntry& entry) {
				func(entry);
			});
		}

		auto find(const Key& key) {
			return m_table.find_if(Hash<Key>{}(key), [&key](const auto& entry) {
				return entry.key() == key;
			});
		}

		Val& operator[](const Key& key) {
			return (*find(key)).value();
		}

		Iter begin() {
			return m_table.tbegin();
		}

		Iter end() {
			return m_table.tend();
		}

		size_t size() { return m_table.size(); }
		size_t bucket_count() { return m_table.bucket_count(); }
	};


}

using ARLib::HashMap;