#pragma once
#include "BaseTraits.hpp"
#include "Concepts.hpp"
#include "Enumerate.hpp"
#include "Iterator.hpp"
#include "PrintInfo.hpp"
#include "TypeTraits.hpp"
#include "Optional.hpp"
#include "Span.hpp"
namespace ARLib {
template <typename VT>
class GenericView {
    using T         = RemoveReferenceT<VT>;
    T* m_begin_view = nullptr;
    T* m_end_view   = nullptr;

    public:
    constexpr GenericView(T* begin, T* end) : m_begin_view(begin), m_end_view(end) {}
    constexpr GenericView(T* begin, size_t size) : m_begin_view(begin), m_end_view(begin + size) {}
    // this constructor works only if the container operates on contiguous memory (e.g. not a linked list)
    // actually this whole class only operates on contiguous memory
    constexpr explicit GenericView(Iterable auto& container) {
        m_begin_view = &(*container.begin());
        m_end_view   = &(*container.end());
    }
    constexpr size_t size() const { return static_cast<size_t>(m_end_view - m_begin_view); }
    constexpr Iterator<T> begin() { return Iterator<T>{ m_begin_view }; }
    constexpr Iterator<T> end() { return Iterator<T>{ m_end_view }; }
    constexpr ReverseIterator<T> rbegin() { return ReverseIterator<T>{ m_end_view - 1 }; }
    constexpr ReverseIterator<T> rend() { return ReverseIterator<T>{ m_begin_view - 1 }; }
    constexpr ConstIterator<T> begin() const { return ConstIterator<T>{ m_begin_view }; }
    constexpr ConstIterator<T> end() const { return ConstIterator<T>{ m_end_view }; }
    constexpr ConstReverseIterator<T> rbegin() const { return ConstReverseIterator<T>{ m_end_view - 1 }; }
    constexpr ConstReverseIterator<T> rend() const { return ConstReverseIterator<T>{ m_begin_view - 1 }; }
    constexpr T& operator[](size_t index) { return m_begin_view[index]; }
    constexpr const T& operator[](size_t index) const { return m_begin_view[index]; }
    constexpr const T* data() const { return m_begin_view; }
    constexpr T* data() { return m_begin_view; }
    constexpr auto span() const { return Span<const T>{ m_begin_view, m_end_view }; }
    constexpr auto span() { return Span<T>{ m_begin_view, m_end_view }; }
    template <typename Functor>
    constexpr void for_each(Functor func) {
        for (auto& item : *this) { func(item); }
    }
};
template <Iterable Cont>
GenericView(Cont&) -> GenericView<ContainerValueTypeT<Cont>>;

template <Iterable Cont>
GenericView(const Cont&) -> GenericView<AddConstT<ContainerValueTypeT<Cont>>>;

template <typename T>
using ReadOnlyView = GenericView<AddConstT<T>>;
template <Iterable Cont>
class IteratorView {
    using Iter         = decltype(declval<Cont>().begin());
    using IterRet      = ConditionalT<CopyConstructible<Iter>, Iter, AddLvalueReferenceT<Iter>>;
    using ConstIterRet = AddConstT<IterRet>;
    Iter m_begin;
    Iter m_end;
    using ItemType             = RemoveReferenceT<IteratorInputType<Iter>>;
    using OutputType           = RemoveReferenceT<IteratorOutputType<Iter>>;
    using FindOutputType       = IteratorOutputType<Iter>;
    ItemType* m_stolen_storage = nullptr;
    ItemType* release_storage() {
        ItemType* storage = m_stolen_storage;
        m_stolen_storage  = nullptr;
        return storage;
    }

    public:
    IteratorView(const IteratorView& other) = delete;
    IteratorView(IteratorView&& other) noexcept :
        m_begin(move(other.m_begin)), m_end(move(other.m_end)), m_stolen_storage(other.release_storage()) {}
    IteratorView(ItemType* storage, Iter begin, Iter end) :
        m_begin(move(begin)), m_end(move(end)), m_stolen_storage(storage) {}
    IteratorView(ItemType* storage, size_t size) : m_begin(storage), m_end(storage + size), m_stolen_storage(storage) {}
    explicit IteratorView(Cont& cont) : m_begin(cont.begin()), m_end(cont.end()) {}
    IterRet begin() { return m_begin; }
    IterRet end() { return m_end; }
    ConstIterRet begin() const { return m_begin; }
    ConstIterRet end() const { return m_end; }
    size_t size()
    requires IterCanSubtractForSize<Iter>
    {
        return m_end - m_begin;
    }
    auto skip(size_t n) {
        if constexpr (IterCanAdvanceWithOffset<Iter> && IterCanSubtractForSize<Iter>) {
            size_t size = m_end - m_begin;
            if (n > size) { n = size; }
            return IteratorView{ release_storage(), m_begin + n, m_end };
        } else {
            auto new_begin = m_begin;
            for (size_t i = 0; i < n && new_begin != m_end; ++i) { ++new_begin; }
            return IteratorView{ release_storage(), new_begin, m_end };
        }
    }
    template <typename NewCont>
    requires Pushable<NewCont, IteratorOutputType<Iter>>
    NewCont join() {
        NewCont collector{};
        for (auto it = m_begin; it != m_end; ++it) { collector.append(*it); }
        return collector;
    }
    template <typename NewCont, Constructible<IteratorOutputType<Iter>> JoinObj>
    requires Pushable<NewCont, IteratorOutputType<Iter>>
    NewCont join_with(JoinObj obj) {
        using Ot = IteratorOutputType<Iter>;
        --m_end;    // move end one before
        auto joiner = Ot{ Forward<JoinObj>(obj) };
        NewCont collector{};
        for (auto it = m_begin; it != m_end; ++it) {
            collector.append(*it);
            collector.append(joiner);
        }
        collector.append(*m_end);
        ++m_end;
        return collector;
    }
    template <typename NewCont = Cont>
    NewCont collect()
    requires Pushable<NewCont, IteratorOutputType<Iter>>
    {
        if (m_stolen_storage != nullptr) {
            if constexpr (SameAs<NewCont, Cont> && IterCanSubtractForSize<Iter>) {
                return NewCont{ m_stolen_storage, size() };
            } else {
                NewCont copy{};
                if constexpr (Reservable<NewCont> && IterCanSubtractForSize<Iter>) { copy.reserve(size()); }
                for (auto it = m_begin; it != m_end; ++it) { copy.append(move(*it)); }
                delete[] m_stolen_storage;
                m_stolen_storage = nullptr;
                return copy;
            }
        } else {
            NewCont copy{};
            if constexpr (Reservable<NewCont> && IterCanSubtractForSize<Iter>) { copy.reserve(size()); }
            for (auto it = m_begin; it != m_end; ++it) { copy.append(move(*it)); }
            return copy;
        }
    }
    template <template <typename> typename NewCont, typename ItemT = RemoveCvRefT<IteratorOutputType<Iter>>>
    NewCont<ItemT> collect()
    requires Pushable<NewCont<ItemT>, ItemT>
    {
        using RealCont = NewCont<ItemT>;
        if (m_stolen_storage != nullptr) {
            if constexpr (SameAs<RealCont, Cont> && IterCanSubtractForSize<Iter>) {
                return RealCont{ m_stolen_storage, size() };
            } else {
                RealCont copy{};
                if constexpr (Reservable<RealCont> && IterCanSubtractForSize<Iter>) { copy.reserve(size()); }
                for (auto it = m_begin; it != m_end; ++it) { copy.append(move(*it)); }
                deallocate<ItemType, DeallocType::Multiple>(m_stolen_storage);
                m_stolen_storage = nullptr;
                return copy;
            }
        } else {
            RealCont copy{};
            if constexpr (Reservable<RealCont> && IterCanSubtractForSize<Iter>) { copy.reserve(size()); }
            for (auto it = m_begin; it != m_end; ++it) { copy.append(move(*it)); }
            return copy;
        }
    }
    template <typename Functor>
    auto filter(Functor func) {
        auto filter_iter = FilterIterate{ *this, func };
        return IteratorView<decltype(filter_iter)>{ release_storage(), filter_iter.begin(), filter_iter.end() };
    }
    template <typename Functor>
    auto map(Functor func) {
        auto map_iter = MapIterate{ *this, func };
        return IteratorView<decltype(map_iter)>{ release_storage(), map_iter.begin(), map_iter.end() };
    }
    template <typename T2>
    requires requires {
        { T2{ declval<IteratorOutputType<Iter>>() } } -> SameAs<T2>;
    }
    auto map() {
        auto conversion_func = [](const auto& v) {
            return T2{ v };
        };
        auto map_iter = MapIterate{ *this, conversion_func };
        return IteratorView<decltype(map_iter)>{ release_storage(), map_iter.begin(), map_iter.end() };
    }
    auto enumerate() {
        auto enum_iter = Enumerate{ *this };
        return IteratorView<decltype(enum_iter)>{ release_storage(), enum_iter.begin(), enum_iter.end() };
    }
    template <Iterable C>
    auto zip(C&& cont) & {
        return ARLib::zip(*this, Forward<C>(cont));
    }
    template <Iterable C>
    auto zip(C&& cont) && {
        return ARLib::zip(move(*this), Forward<C>(cont));
    }
    template <typename Functor>
    requires(CallableWithRes<Functor, ItemType, ItemType>)
    IteratorView inplace_transform(Functor&& func) {
        for (auto it = m_begin; it != m_end; ++it) { *it = func(*it); }
        return move(*this);
    }
    constexpr auto max() const { return ARLib::max(*this); }
    constexpr auto min() const { return ARLib::min(*this); }
    constexpr auto sum() const { return ARLib::sum(begin(), end()); }
    template <typename Functor>
    requires CallableWith<Functor, OutputType>
    constexpr auto for_each(Functor f) {
        for (auto it = m_begin; it != m_end; ++it) { invoke(Forward<Functor>(f), *it); }
    }
    template <typename Functor>
    requires CallableWithRes<Functor, OutputType, bool>
    constexpr auto all(Functor f) {
        for (auto it = m_begin; it != m_end; ++it) {
            if (!invoke(Forward<Functor>(f), *it)) return false;
        }
        return true;
    }
    template <typename Functor>
    requires CallableWithRes<Functor, OutputType, bool>
    constexpr auto any(Functor f) {
        for (auto it = m_begin; it != m_end; ++it) {
            if (invoke(Forward<Functor>(f), *it)) return true;
        }
        return false;
    }
    template <typename Functor>
    requires CallableWithRes<Functor, OutputType, bool>
    constexpr Optional<FindOutputType> find_if(Functor f) {
        for (auto it = m_begin; it != m_end; ++it) {
            if (invoke(Forward<Functor>(f), *it)) return { *it };
        }
        return {};
    }
    constexpr Optional<FindOutputType> find(const OutputType& val) {
        for (auto it = m_begin; it != m_end; ++it) {
            if (*it == val) return { *it };
        }
        return {};
    }
    ~IteratorView() { deallocate<ItemType, DeallocType::Multiple>(m_stolen_storage); }
};
template <Printable T>
struct PrintInfo<GenericView<T>> {
    const GenericView<T>& m_view;
    PrintInfo(const GenericView<T>& view) : m_view(view) {}
    String repr() const {
        if (m_view.size() == 0) return "[]"_s;
        String view_str{ "[ " };
        for (const auto& val : m_view) {
            view_str += PrintInfo<T>{ val }.repr();
            view_str += ", "_s;
        }
        view_str = view_str.substring(0, view_str.size() - 2);
        view_str += " ]"_s;
        return view_str;
    }
};
}    // namespace ARLib
