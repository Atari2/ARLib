#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "HashBase.h"
#include "HashTable.h"
#include "PrintInfo.h"

namespace ARLib {

    template <Hashable Key, typename Val>
    struct HashMapEntry {
        Key m_key;
        Val m_value;
        size_t m_hashval;

        public:
        HashMapEntry() : m_key(), m_value(), m_hashval(static_cast<size_t>(-1)) {}
        HashMapEntry(const Key& key, const Val& value) : m_key(key), m_value(value) { m_hashval = Hash<Key>{}(m_key); }
        HashMapEntry(Key&& key, Val&& value) : m_key(move(key)), m_value(move(value)) {
            m_hashval = Hash<Key>{}(m_key);
        }
        HashMapEntry(HashMapEntry&& other) noexcept : m_key(move(other.m_key)), m_value(move(other.m_value)) {
            m_hashval = other.m_hashval;
        }
        HashMapEntry(const HashMapEntry& other) : m_key(other.m_key), m_value(other.m_value) {
            m_hashval = other.m_hashval;
        }
        HashMapEntry& operator=(const HashMapEntry& other) {
            m_key = other.m_key;
            m_value = other.m_value;
            m_hashval = other.m_hashval;
            return *this;
        }

        HashMapEntry& operator=(HashMapEntry&& other) noexcept {
            m_key = move(other.m_key);
            m_value = move(other.m_value);
            m_hashval = other.m_hashval;
            return *this;
        }

        constexpr size_t hashval() const { return m_hashval; }

        const Key& key() const { return m_key; }
        const Val& value() const { return m_value; }

        Val& value() { return m_value; }

        bool operator==(const HashMapEntry& other) const { return m_hashval == other.m_hashval; }

        bool operator!=(const HashMapEntry& other) { return m_hashval != other.m_hashval; }
    };

    template <typename Key, typename Value>
    struct Hash<HashMapEntry<Key, Value>> {
        [[nodiscard]] forceinline size_t operator()(const HashMapEntry<Key, Value>& key) const noexcept {
            return key.hashval();
        }
    };

    template <Hashable Key, typename Val>
    class HashMap {
        using MapEntry = HashMapEntry<Key, Val>;
        using Iter = HashTableIterator<MapEntry>;
        using ConstIter = ConstHashTableIterator<MapEntry>;
        HashTable<MapEntry> m_table{};

        public:
        HashMap() = default;

        HashMap(std::initializer_list<MapEntry> entries) {
            for (auto val : entries) {
                add(move(val));
            }
        }

        HashMap(const HashMap& other) : m_table(other.m_table) {}
        HashMap(HashMap&& other) noexcept : m_table(move(other.m_table)) {}
        HashMap& operator=(HashMap&& other) noexcept {
            m_table = move(other.m_table);
            return *this;
        }
        HashMap& operator=(const HashMap& other) {
            m_table = other.m_table;
            return *this;
        }
        double load() { return m_table.load(); };

        InsertionResult add(const Key& key, const Val& value) requires(CopyAssignable<Key>&& CopyAssignable<Val>) {
            MapEntry entry{key, value};
            return m_table.insert(MapEntry{key, value});
        }
        InsertionResult add(Key&& key, Val&& value) requires(MoveAssignable<Key>&& MoveAssignable<Val>) {
            MapEntry entry{Forward<Key>(key), Forward<Val>(value)};
            return m_table.insert(Forward<MapEntry>(entry));
        }
        InsertionResult add(MapEntry entry) { return m_table.insert(Forward<MapEntry>(entry)); }

        DeletionResult remove(Key key) {
            MapEntry entry{Forward<Key>(key), Val()};
            return m_table.remove(entry);
        }

        template <typename Functor>
        void for_each(Functor func) {
            m_table.for_each(func);
        }

        void for_each(void (*func)(const MapEntry&)) const { m_table.for_each(func); }

        Iter find(const Key& key) {
            return m_table.find_if(Hash<Key>{}(key), [&key](const auto& entry) { return entry.key() == key; });
        }

        ConstIter find(const Key& key) const {
            return m_table.find_if(Hash<Key>{}(key), [&key](const auto& entry) { return entry.key() == key; });
        }

        Val& operator[](const Key& key) { return (*find(key)).value(); }
        const Val& operator[](const Key& key) const { return (*find(key)).value(); }

        Iter begin() { return m_table.tbegin(); }

        Iter end() { return m_table.tend(); }

        ConstIter begin() const { return m_table.tbegin(); }

        ConstIter end() const { return m_table.tend(); }

        size_t size() const { return m_table.size(); }
        size_t bucket_count() const { return m_table.bucket_count(); }
    };

    template <typename A, typename B>
    struct PrintInfo<HashMapEntry<A, B>> {
        const HashMapEntry<A, B>& m_entry;
        explicit PrintInfo(const HashMapEntry<A, B>& entry) : m_entry(entry) {}
        String repr() const {
            return "HashMapEntry { "_s + print_conditional<A>(m_entry.key()) + ": "_s +
                   print_conditional<B>(m_entry.value()) + ", hash: " + print_conditional<size_t>(m_entry.hashval()) +
                   " }"_s;
        }
    };

    template <Printable A, Printable B>
    struct PrintInfo<HashMap<A, B>> {
        const HashMap<A, B>& m_map;
        explicit PrintInfo(const HashMap<A, B>& map) : m_map(map) {}
        String repr() const {
            if (m_map.size() == 0) { return "{}"_s; }
            String con{};
            con.append("{ ");
            for (const auto& entry : m_map) {
                con.append(PrintInfo<A>{entry.key()}.repr());
                con.append(": "_s);
                con.append(PrintInfo<B>{entry.value()}.repr());
                con.append(", ");
            }
            return con.substring(0, con.size() - 2) + " }"_s;
        }
    };

} // namespace ARLib
