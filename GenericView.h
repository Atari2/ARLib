#pragma once
#include "BaseTraits.h"
#include "Concepts.h"
#include "Enumerate.h"
#include "Iterator.h"
#include "PrintInfo.h"
#include "TypeTraits.h"
namespace ARLib {
template <typename T>
class GenericView {
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
    template <typename Functor>
    constexpr void for_each(Functor func) {
        for (auto& item : *this) { func(item); }
    }
};
template <Iterable Cont>
GenericView(Cont&) -> GenericView<typename IterableTraits<Cont>::ItemType>;

template <Iterable Cont>
GenericView(const Cont&) -> GenericView<AddConstT<typename IterableTraits<Cont>::ItemType>>;

template <typename T>
using ReadOnlyView = GenericView<AddConstT<T>>;
template <Iterable Cont>
class IteratorView {
    using Iter = decltype(declval<Cont>().begin());
    Iter m_begin;
    Iter m_end;
    using ItemType             = RemoveReferenceT<decltype(*declval<Iter>())>;
    ItemType* m_stolen_storage = nullptr;
    ItemType* release_storage() {
        ItemType* storage = m_stolen_storage;
        m_stolen_storage  = nullptr;
        return storage;
    }

    public:
    IteratorView(const IteratorView& other) = delete;
    IteratorView(IteratorView&& other) : m_begin(other.m_begin), m_end(other.m_end), m_stolen_storage(other.release_storage()) {}
    IteratorView(ItemType* storage, Iter begin, Iter end) : m_begin(begin), m_end(end), m_stolen_storage(storage) {}
    IteratorView(ItemType* storage, size_t size) : m_begin(storage), m_end(storage + size), m_stolen_storage(storage) {}
    explicit IteratorView(Cont& cont) : m_begin(cont.begin()), m_end(cont.end()) {}
    Iter begin() { return m_begin; }
    Iter end() { return m_end; }
    size_t size()
    requires IterCanSubtractForSize<Iter>
    {
        return m_end - m_begin;
    }
    // in-place transform
    template <typename Functor>
    requires(IsSameV<decltype(declval<Functor>()(declval<ItemType>())), ItemType>)
    IteratorView transform(Functor&& func) {
        for (auto it = m_begin; it != m_end; ++it) { *it = func(*it); }
        return move(*this);
    }
    template <typename NewCont = Cont>
    NewCont collect()
    requires Pushable<NewCont, ItemType>
    {
        if (m_stolen_storage != nullptr) {
            if constexpr (SameAs<NewCont, Cont> && IterCanSubtractForSize<Iter>) {
                return NewCont{ m_stolen_storage, size() };
            } else {
                NewCont copy{};
                if constexpr (Reservable<NewCont> && IterCanSubtractForSize<Iter>) { copy.reserve(size()); }
                for (auto it = m_begin; it != m_end; ++it) { copy.push_back(move(*it)); }
                delete[] m_stolen_storage;
                m_stolen_storage = nullptr;
                return copy;
            }
        } else {
            NewCont copy{};
            if constexpr (Reservable<NewCont> && IterCanSubtractForSize<Iter>) { copy.reserve(size()); }
            for (auto it = m_begin; it != m_end; ++it) { copy.push_back(*it); }
            return copy;
        }
    }
    template <typename Functor>
    auto filter(Functor func) {
        auto filter_iter = FilterIterate{ *this, func };
        return IteratorView<decltype(filter_iter)>{ release_storage(), filter_iter.begin(), filter_iter.end() };
    }
    template <typename Functor, typename NewCont = Cont, typename Type = ResultOfT<Functor(ItemType)>>
    auto map_view(Functor func) {
        if constexpr (IsSameV<Type, ItemType>) {
            static_assert(IsSameV<Cont, NewCont>);
            Type* new_storage = m_stolen_storage != nullptr ? m_stolen_storage : new Type[size()];
            size_t index      = 0;
            for (auto it = m_begin; it != m_end; ++it) {
                new_storage[index] = move(func(*it));
                index++;
            }
            if (m_stolen_storage != nullptr) m_stolen_storage = nullptr;
            return IteratorView<NewCont>{ new_storage, size() };
        } else {
            Type* new_storage = new Type[size()];
            size_t index      = 0;
            for (auto it = m_begin; it != m_end; ++it) {
                new_storage[index] = move(func(*it));
                index++;
            }
            if (m_stolen_storage != nullptr) {
                delete[] m_stolen_storage;
                m_stolen_storage = nullptr;
            }
            return IteratorView<NewCont>{ new_storage, size() };
        }
    }
    template <typename Functor>
    Cont map(Functor func)
    requires Pushable<Cont, ItemType> && SameAs<ResultOfT<Functor(ItemType)>, ItemType>
    {
        Cont copy{};
        if constexpr (Reservable<Cont>) { copy.reserve(size()); }
        for (auto it = m_begin; it != m_end; ++it) { copy.push_back(func(*it)); }
        return copy;
    }
    template <Iterable OtherCont, typename Functor>
    OtherCont transform_map(Functor func)
    requires Pushable<OtherCont, ResultOfT<Functor(ItemType)>>
    {
        OtherCont cont{};
        if constexpr (Reservable<OtherCont>) { cont.reserve(size()); }
        for (auto it = m_begin; it != m_end; ++it) { cont.push_back(func(*it)); }
        return cont;
    }
    ~IteratorView() { delete[] m_stolen_storage; }
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
