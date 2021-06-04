#pragma once
#include "Vector.h"
#include "Concepts.h"
#include "HashTable.h"

namespace ARLib {
	template <EqualityComparable Key, typename Val>
	class MapEntry {
		Key m_key;
		Val m_value;

	public:
		MapEntry() : m_key(), m_value() {

		}
		MapEntry(Key key, Val value) : m_key(move(key)), m_value(move(value)) {
		}
		MapEntry(MapEntry&& other) noexcept : m_key(move(other.m_key)), m_value(move(other.m_value)) {
		}
		MapEntry(const MapEntry& other) : m_key(other.m_key), m_value(other.m_value) {
		}

		MapEntry& operator=(MapEntry&& other) noexcept {
			m_key = move(other.m_key);
			m_value = move(other.m_value);
			return *this;
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

		bool operator==(const MapEntry& other) {
			return m_key == other.m_key;
		}

		bool operator!=(const MapEntry& other) {
			return m_key != other.m_key;
		}
	};

	template <EqualityComparable Key, typename Val>
	class Map {
		using Entry = MapEntry<Key, Val>;
		Vector<Entry> m_storage{};
	public:
		Map() = default;

		InsertionResult add(Key key, Val value) {
			Entry entry{ key, value };
			auto it = find(key);
			if (it == m_storage.end()) {
				m_storage.append(Forward<Entry>(entry));
				return InsertionResult::New;
			}
			else {
				(*it) = move(entry);
				return InsertionResult::Replace;
			}
		}

		InsertionResult add(Entry entry) {
			auto it = find(entry.key());
			if (it == m_storage.end()) {
				m_storage.append(Forward<Entry>(entry));
				return InsertionResult::New;
			}
			else {
				(*it) = move(entry);
				return InsertionResult::Replace;
			}
		}

		template <typename Functor>
		void for_each(Functor func) {
			m_storage.for_each([&func](const Entry& entry) {
				func(entry);
			});
		}

		auto find(const Key& key) {
			for (auto it = m_storage.begin(); it != m_storage.end(); it++)
				if ((*it).key() == key)
					return it;
			return m_storage.end();
		}

		Val& operator[](const Key& key) {
			return (*find(key)).value();
		}

		size_t size() { return m_storage.size(); }
	};

}

using ARLib::Map;
