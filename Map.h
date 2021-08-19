#pragma once
#include "Concepts.h"
#include "HashTable.h"
#include "Vector.h"
#include "PrintInfo.h"

namespace ARLib {
    template <EqualityComparable Key, typename Val>
    struct MapEntry {
        Key m_key;
        Val m_value;

        MapEntry() : m_key(), m_value() {}
        MapEntry(const Key& key, const Val& value) : m_key(key), m_value(value) {}
        MapEntry(Key&& key, Val&& value) : m_key(move(key)), m_value(move(value)) {}
        MapEntry(MapEntry&& other) noexcept : m_key(move(other.m_key)), m_value(move(other.m_value)) {}
        MapEntry(const MapEntry& other) : m_key(other.m_key), m_value(other.m_value) {}

        MapEntry& operator=(MapEntry&& other) noexcept {
            if (this == &other) return *this;
            m_key = move(other.m_key);
            m_value = move(other.m_value);
            return *this;
        }

        MapEntry& operator=(const MapEntry& other) {
            if (this == &other) return *this;
            m_key = other.m_key;
            m_value = other.m_value;
            return *this;
        }

        const Key& key() const { return m_key; }
        const Val& value() const { return m_value; }

        Val& value() { return m_value; }

        bool operator==(const MapEntry& other) { return m_key == other.m_key; }

        bool operator!=(const MapEntry& other) { return m_key != other.m_key; }
    };

    template <EqualityComparable Key, typename Val>
    class Map {
        using Entry = MapEntry<Key, Val>;
        Vector<Entry> m_storage{};

        public:
        Map() = default;

        Map(std::initializer_list<Entry> list) {
            m_storage.reserve(list.size());
            for (auto val : list) {
                add(move(val));
            }
        }

        InsertionResult add(Key key, Val value) {
            Entry entry{key, value};
            auto it = find(key);
            if (it == m_storage.end()) {
                m_storage.append(Forward<Entry>(entry));
                return InsertionResult::New;
            } else {
                (*it) = move(entry);
                return InsertionResult::Replace;
            }
        }

        InsertionResult add(Entry entry) {
            auto it = find(entry.key());
            if (it == m_storage.end()) {
                m_storage.append(Forward<Entry>(entry));
                return InsertionResult::New;
            } else {
                (*it) = move(entry);
                return InsertionResult::Replace;
            }
        }

        template <typename Functor>
        void for_each(Functor func) {
            m_storage.for_each([&func](const Entry& entry) { func(entry); });
        }

        auto find(const Key& key) {
            for (auto it = m_storage.begin(); it != m_storage.end(); it++)
                if ((*it).key() == key) return it;
            return m_storage.end();
        }

        Val& operator[](const Key& key) { return (*find(key)).value(); }

        size_t size() const { return m_storage.size(); }

        Iterator<Entry> begin() { return m_storage.begin(); }
        ConstIterator<Entry> begin() const { return m_storage.cbegin(); }

        ConstIterator<Entry> cbegin() const { return m_storage.cbegin(); }

        Iterator<Entry> end() { return m_storage.end(); }
        ConstIterator<Entry> end() const { return m_storage.cend(); }

        ConstIterator<Entry> cend() { return m_storage.cend(); }
    };

    template <Printable A, Printable B>
    struct PrintInfo<Map<A, B>> {
        const Map<A, B>& m_map;
        PrintInfo(const Map<A, B>& map) : m_map(map) {}
        String repr() const {
            if (m_map.size() == 0) { return "{}"_s; }
            String con{};
            con.concat("{ ");
            for (const auto& [key, val] : m_map) {
                con.concat(PrintInfo<A>{key}.repr());
                con.concat(": "_s);
                con.concat(PrintInfo<B>{val}.repr());
                con.concat(", ");
            }
            return con.substring(0, con.size() - 2) + " }"_s;
        }
    };

} // namespace ARLib

using ARLib::Map;
