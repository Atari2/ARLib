#pragma once
#include "HashBase.hpp"
#include "Concepts.hpp"
#include "Vector.hpp"
#include "FlatSet.hpp"
namespace ARLib {
template <typename Key, typename Val, typename HashCls = Hash<Key>>
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
    template <typename O>
    requires(!SameAs<RemoveCvRefT<O>, FlatMapEntry<Key, Val, HashCls>>)
    friend bool operator==(const FlatMapEntry<Key, Val, HashCls>& lhs, O&& rhs) {
        return lhs.m_key == rhs;
    }
    template <typename O>
    requires(!SameAs<RemoveCvRefT<O>, FlatMapEntry<Key, Val, HashCls>>)
    friend bool operator!=(const FlatMapEntry<Key, Val, HashCls>& lhs, O&& rhs) {
        return lhs.m_key != rhs;
    }
    template <typename O>
    requires(!SameAs<RemoveCvRefT<O>, FlatMapEntry<Key, Val, HashCls>>)
    friend bool operator==(O&& lhs, const FlatMapEntry<Key, Val, HashCls>& rhs) {
        return lhs == rhs.m_key;
    }
    template <typename O>
    requires(!SameAs<RemoveCvRefT<O>, FlatMapEntry<Key, Val, HashCls>>)
    friend bool operator!=(O&& lhs, const FlatMapEntry<Key, Val, HashCls>& rhs) {
        return lhs != rhs.m_key;
    }
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
template <typename Key, typename Val, typename HashCls>
requires Hashable<Key, HashCls>
struct Hash<FlatMapEntry<Key, Val, HashCls>> {
    HashCls m_hasher;
    size_t operator()(const FlatMapEntry<Key, Val, HashCls>& key) const { return m_hasher(key.key()); }
};
template <typename Key, typename Val, typename HashCls = Hash<Key>>
requires Hashable<Key, HashCls>
class FlatMap {
    using Entry = FlatMapEntry<Key, Val, HashCls>;
    FlatSet<Entry> m_table{};

    public:
    using ValueType = Entry;
    FlatMap()       = default;
    FlatMap(std::initializer_list<Entry> entry) {
        for (auto& e : entry) { m_table.insert(Entry{ e }); }
    }
    auto find(const Key& value) const { return m_table.find(value); }
    // support heterogeneous lookup
    template <typename O, typename OHashCls = Hash<RemoveCvRefT<O>>>
    requires(EqualityComparableWith<O, Key> && Hashable<O, OHashCls>)
    auto find(O&& value) const {
        return m_table.template find<O, OHashCls>(Forward<O>(value));
    }
    auto begin() const { return m_table.begin(); }
    auto end() const { return m_table.end(); }
    bool contains(const Key& value) const { return find(value) != end(); }
    bool remove(const Key& value) { return m_table.remove(value); }
    auto insert(Entry&& entry) { return m_table.insert(Forward<Entry>(entry)); }
    auto insert(Key&& key, Val&& value) { return insert(Entry{ Forward<Key>(key), Forward<Val>(value) }); }
    template <typename... Args>
    requires Constructible<Entry, Args...>
    auto insert(Args&&... args) {
        return m_table.insert(Entry{ Forward<Args>(args)... });
    }
    Val& get_or_default(const Key& key)
    requires DefaultConstructible<Val>
    {
        auto entry                  = Entry{ Key{ key }, Val{} };
        auto&& [is_not_present, it] = m_table.__hashmap_private_prepare_for_insert(entry);
        if (is_not_present) { return m_table.__hashmap_private_insert(it, move(entry)).val(); }
        return const_cast<Val&>((*it).val());
    }
    template <typename O, typename OHashCls = Hash<RemoveCvRefT<O>>>
    requires(EqualityComparableWith<O, Key> && Hashable<O, OHashCls> && !SameAsCvRef<O, Key> && Constructible<Key, O>)
    Val& get_or_default(O&& key)
    requires DefaultConstructible<Val>
    {
        auto entry                     = Entry{ Key{ key }, Val{} };
        auto&& [is_not_present, it] = m_table.__hashmap_private_prepare_for_insert(entry);
        if (is_not_present) { return m_table.__hashmap_private_insert(it, move(entry)).val(); }
        return const_cast<Val&>((*it).val());
    }
    Val& get_or_insert(const Key& key, Val&& val) {
        auto entry                  = Entry{ Key{ key }, Forward<Val>(val) };
        auto&& [is_not_present, it] = m_table.__hashmap_private_prepare_for_insert(entry);
        if (is_not_present) { return m_table.__hashmap_private_insert(it, move(entry)).val(); }
        return const_cast<Val&>((*it).val());
    }
    template <typename O, typename OHashCls = Hash<RemoveCvRefT<O>>>
    requires(EqualityComparableWith<O, Key> && Hashable<O, OHashCls> && !SameAsCvRef<O, Key> && Constructible<Key, O>)
    Val& get_or_insert(O&& key, Val&& val) {
        auto entry                  = Entry{ Key{ key }, Forward<Val>(val) };
        auto&& [is_not_present, it] = m_table.__hashmap_private_prepare_for_insert(entry);
        if (is_not_present) { return m_table.__hashmap_private_insert(it, move(entry)).val(); }
        return const_cast<Val&>((*it).val());
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
    template <typename O, typename OHashCls = Hash<RemoveCvRefT<O>>>
    requires(EqualityComparableWith<O, Key> && Hashable<O, OHashCls> && !SameAsCvRef<O, Key>)
    Val& operator[](O&& value) {
        auto it = m_table.template find<O, OHashCls>(Forward<O>(value));
        HARD_ASSERT(it != m_table.end(), "FlatMap::operator[] failed to find key");
        return const_cast<Entry&>((*it)).val();
    }
    template <typename O, typename OHashCls = Hash<RemoveCvRefT<O>>>
    requires(EqualityComparableWith<O, Key> && Hashable<O, OHashCls> && !SameAsCvRef<O, Key>)
    const Val& operator[](O&& value) const {
        auto it = m_table.template find<O, OHashCls>(Forward<O>(value));
        HARD_ASSERT(it != m_table.end(), "FlatMap::operator[] failed to find key");
        return (*it).val();
    }
    size_t size() const { return m_table.size(); }
};
template <typename A, typename B, typename H>
struct PrintInfo<FlatMapEntry<A, B, H>> {
    const FlatMapEntry<A, B, H>& m_entry;
    explicit PrintInfo(const FlatMapEntry<A, B, H>& entry) : m_entry(entry) {}
    String repr() const {
        return "{ "_s + print_conditional<A>(m_entry.key()) + ": "_s + print_conditional<B>(m_entry.val()) + " }"_s;
    }
};
template <Printable A, Printable B, typename H>
struct PrintInfo<FlatMap<A, B, H>> {
    const FlatMap<A, B, H>& m_map;
    explicit PrintInfo(const FlatMap<A, B, H>& map) : m_map(map) {}
    String repr() const {
        if (m_map.size() == 0) { return "{}"_s; }
        String con{};
        con.append("{ ");
        for (const auto& [k, v] : m_map) {
            con.append(PrintInfo<A>{ k }.repr());
            con.append(": "_s);
            con.append(PrintInfo<B>{ v }.repr());
            con.append(", ");
        }
        return con.substring(0, con.size() - 2) + " }"_s;
    }
};
}    // namespace ARLib
template <typename K, typename V, typename H>
struct std::tuple_size<ARLib::FlatMapEntry<K, V, H>> : ARLib::IntegralConstant<ARLib::size_t, 2> {};
template <typename K, typename V, typename H>
struct std::tuple_size<const ARLib::FlatMapEntry<K, V, H>> : ARLib::IntegralConstant<ARLib::size_t, 2> {};
template <typename K, typename V, typename H>
struct std::tuple_element<0, ARLib::FlatMapEntry<K, V, H>> {
    using type = K;
};
template <typename K, typename V, typename H>
struct std::tuple_element<1, ARLib::FlatMapEntry<K, V, H>> {
    using type = V;
};
template <typename K, typename V, typename H>
struct std::tuple_element<0, const ARLib::FlatMapEntry<K, V, H>> {
    using type = const K;
};
template <typename K, typename V, typename H>
struct std::tuple_element<1, const ARLib::FlatMapEntry<K, V, H>> {
    using type = const V;
};