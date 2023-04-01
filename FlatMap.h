#pragma once
#include "HashBase.h"
#include "Concepts.h"
#include "Vector.h"
#include "FlatSet.h"
namespace ARLib {
template <Hashable Key, typename Val>
class FlatMapEntry {
    Key m_key;
    Val m_val;
    public:
    FlatMapEntry(Key&& k, Val&& val) : m_key{ Forward<Key>(k) }, m_val{ Forward<Val>(val) } {}
    template <typename KH, typename VH>
    requires(Constructible<Key, KH> && Constructible<Val, VH>)
    FlatMapEntry(KH&& k, VH&& val) : m_key{ Forward<KH>(k) }, m_val{ Forward<VH>(val) } {}
    bool operator==(const FlatMapEntry& other) const { return m_key == other.m_key; }
    bool operator==(const Key& key) const { return m_key == key; }
    const Key& key() const { return m_key; }
    const Val& val() const { return m_val; }
    Val& val() { return m_val; }
    template <size_t I>
    requires(I == 0 || I == 1)
    const auto& get() const {
        if constexpr (I == 0) {
            return m_key;
        } else {
            return m_val;
        }
    }
};
template <Hashable Key, typename Val>
struct Hash<FlatMapEntry<Key, Val>> {
    size_t operator()(const FlatMapEntry<Key, Val>& key) const { return Hash<Key>{}(key.key()); }
};
template <Hashable Key, typename Val, typename HashCls = Hash<FlatMapEntry<Key, Val>>>
class FlatMap {
    using Entry = FlatMapEntry<Key, Val>;
    FlatSet<Entry, HashCls> m_table{};

    public:
    FlatMap() = default;
    auto find(const Key& value) const { return m_table.find(value); }
    // template <Hashable O>
    // requires EqualityComparableWith<O, Key>
    // auto find(O&& value) const {
    //     return m_table.find(Forward<O>(value));
    // }
    auto begin() const { return m_table.begin(); }
    auto end() const { return m_table.end(); }
    bool contains(const Key& value) const { return find(value) != end(); }
    bool remove(const Key& value) { return m_table.remove(value); }
    bool insert(Entry&& entry) { return m_table.insert(Forward<Entry>(entry)); }
    bool insert(Key&& key, Val&& value) { return insert(Entry{ Forward<Key>(key), Forward<Val>(value) }); }
    template <typename... Args>
    requires Constructible<Entry, Args...>
    auto insert(Args&&... args) {
        return insert(Entry{ Forward<Args>(args)... });
    }
    auto clear() { m_table.clear(); }
    Val& operator[](const Key& key) {
        auto it = m_table.find(key);
        HARD_ASSERT(it != m_table.end(), "FlatMap::operator[] failed to find key");
        return const_cast<Entry&>((*it)).val();
    }
    const Val& operator[](const Key& key) const {
        auto it = m_table.find(key);
        HARD_ASSERT(it != m_table.end(), "FlatMap::operator[] failed to find key");
        return (*it).val();
    }
    size_t size() const { return m_table.size(); }
};
}    // namespace ARLib
template <typename K, typename V>
struct std::tuple_size<ARLib::FlatMapEntry<K, V>> : ARLib::IntegralConstant<ARLib::size_t, 2> {};
template <typename K, typename V>
struct std::tuple_size<const ARLib::FlatMapEntry<K, V>> : ARLib::IntegralConstant<ARLib::size_t, 2> {};
template <typename K, typename V>
struct std::tuple_element<0, ARLib::FlatMapEntry<K, V>> {
    using type = K;
};
template <typename K, typename V>
struct std::tuple_element<1, ARLib::FlatMapEntry<K, V>> {
    using type = V;
};
template <typename K, typename V>
struct std::tuple_element<0, const ARLib::FlatMapEntry<K, V>> {
    using type = const K;
};
template <typename K, typename V>
struct std::tuple_element<1, const ARLib::FlatMapEntry<K, V>> {
    using type = const V;
};