#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "Utility.h"
namespace ARLib {
// LIFO
template <typename T>
class LinkedList {
    struct ListEntry {
        T m_entry;
        ListEntry* m_next;
        ListEntry(T&& entry, ListEntry* next)
        requires MoveConstructible<T>
            : m_entry(move(entry)), m_next(next) {}
        ListEntry(const T& entry, ListEntry* next)
        requires CopyConstructible<T>
            : m_entry(entry), m_next(next) {}
        T& entry() { return m_entry; }
        const T& entry() const { return m_entry; }
        ListEntry* next() { return m_next; }
        const ListEntry* next() const { return m_next; }
        void swap_next(ListEntry* new_next) { m_next = new_next; }
        ListEntry& operator=(const ListEntry& other)
        requires CopyAssignable<T>
        {
            if (this == &other) return *this;
            m_entry = other.m_entry;
            m_next  = other.m_next;
            return *this;
        }
        ListEntry& operator=(ListEntry&& other) noexcept
        requires MoveAssignable<T>
        {
            if (this == &other) return *this;
            m_entry      = move(other.m_entry);
            m_next       = other.m_next;
            other.m_next = nullptr;
            return *this;
        }
    };
    template <typename ListEntryT>
    class LinkedListIterator {
        friend LinkedList<T>;

        ListEntryT* m_current;
        void internal_advance_() {
            HARD_ASSERT(m_current, "m_current must not be nullptr")
            m_current = m_current->next();
        }

        public:
        explicit LinkedListIterator(ListEntryT* current) : m_current(current) {}
        LinkedListIterator(const LinkedListIterator& other) { m_current = other.m_current; }
        LinkedListIterator& operator=(const LinkedListIterator& other) {
            if (this == &other) return *this;
            m_current = other.m_current;
            return *this;
        }
        LinkedListIterator& operator=(LinkedListIterator&& other) noexcept {
            m_current       = other.m_current;
            other.m_current = nullptr;
            return *this;
        }
        const ListEntryT* current() const { return m_current; }
        ListEntryT* current() { return m_current; }
        bool is_done() const { return m_current->next() == nullptr; }
        bool operator==(const LinkedListIterator& other) const { return m_current == other.m_current; }
        bool operator!=(const LinkedListIterator& other) const { return m_current != other.m_current; }
        const auto& operator*() const { return m_current->entry(); }
        auto& operator*() { return m_current->entry(); }
        LinkedListIterator& operator++() {
            internal_advance_();
            return *this;
        }
        LinkedListIterator operator++(int) {
            LinkedListIterator prev{ m_current };
            internal_advance_();
            return prev;
        }
    };
    using Iter      = LinkedListIterator<ListEntry>;
    using ConstIter = LinkedListIterator<AddConstT<ListEntry>>;

    ListEntry* m_head = nullptr;
    size_t m_size     = 0;
    void internal_single_prepend_(T&& value) {
        m_head = new ListEntry(Forward<T>(value), m_head);
        m_size++;
    }

    public:
    using Entry = ListEntry;
    template <typename... Args>
    explicit LinkedList(T&& val, Args&&... values)
    requires AllOfV<T, Args...>
    {
        internal_single_prepend_(Forward<T>(val));
        (internal_single_prepend_(Forward<Args>(values)), ...);
    }
    LinkedList() = default;
    LinkedList(const LinkedList<T>& other) : m_head(other.m_head), m_size(other.m_size) {}
    LinkedList(LinkedList<T>&& other) noexcept : m_head(other.m_head), m_size(other.m_size) {
        other.m_head = nullptr;
        other.m_size = 0;
    }
    LinkedList<T>& operator=(LinkedList<T>&& other) noexcept {
        m_head       = other.m_head;
        m_size       = other.m_size;
        other.m_head = nullptr;
        other.m_size = 0;
        return *this;
    }
    void prepend(const T& value) { internal_single_prepend_(Forward<T>(T{ value })); }
    void append(const T& value) {
        auto* curr = m_head;
        while (curr->next() != nullptr) { curr = curr->next(); }
        curr->swap_next(new ListEntry(value, nullptr));
        m_size++;
    }
    void prepend(T&& value) { internal_single_prepend_(Forward<T>(value)); }
    void append(T&& value) {
        auto* curr = m_head;
        while (curr->next() != nullptr) { curr = curr->next(); }
        curr->swap_next(new ListEntry(value, nullptr));
        m_size++;
    }
    ListEntry* head() { return m_head; }
    ListEntry* last() {
        auto* curr = m_head;
        while (curr->next() != nullptr) { curr = curr->next(); }
        return curr;
    }
    T pop_head() {
        HARD_ASSERT(m_head, "Calling pop_head() on empty list")
        auto* rem = m_head;
        m_head    = m_head->next();
        m_size--;
        T ret{ move(rem->entry()) };
        delete rem;
        return ret;
    }
    T pop() {
        auto* curr = m_head;
        HARD_ASSERT(curr, "Calling pop() on empty list")
        if (curr->next() == nullptr) {
            m_size--;
            m_head = nullptr;
            T ret{ move(curr->entry()) };
            delete curr;
            return ret;
        }
        while (curr->next()->next() != nullptr) { curr = curr->next(); }
        T ret_next{ move(curr->next()->entry()) };
        delete curr->next();
        curr->swap_next(nullptr);
        m_size--;
        return ret_next;
    }
    void remove(const ListEntry* entry) {
        if (!m_head) return;
        if (entry == m_head) {
            auto* del = m_head;
            m_head    = m_head->next();
            m_size--;
            delete del;
        } else {
            auto* prev = m_head;
            auto* del  = prev->next();
            while (del) {
                if (del == entry) {
                    prev->swap_next(del->next());
                    delete del;
                    m_size--;
                    break;
                }
                prev = del;
                del  = prev->next();
            }
        }
    }
    void remove(const T& item) {
        if (!m_head) return;
        Iter iter = find(item);
        if (iter == end()) return;
        const ListEntry* entry = iter.current();
        remove(entry);
    }
    const T& peek() const {
        if (m_size == 0) return {};
        return m_head->entry();
    }
    void clear() {
        while (m_size > 0) pop_head();
    }
    auto begin() { return Iter{ m_head }; }
    auto end() { return Iter{ nullptr }; }
    auto begin() const { return ConstIter{ m_head }; }
    auto end() const { return ConstIter{ nullptr }; }
    size_t size() const { return m_size; }
    template <typename Functor>
    void for_each(Functor&& func) {
        if (!m_head) return;
        ListEntry* curr = m_head;
        while (curr) {
            func(curr->entry());
            curr = curr->next();
        }
    }
    bool exists(const T& value) const
    requires EqualityComparable<T>
    {
        if (m_size == 0) return false;
        for (const auto& val : *this)
            if (val == value) return true;
        return false;
    }
    auto find(const T& value)
    requires EqualityComparable<T>
    {
        if (m_size == 0) return Iter{ nullptr };
        for (Iter beg = begin(); beg != end(); ++beg) {
            if (*beg == value) return Iter{ beg.m_current };
        }
        return Iter{ nullptr };
    }
    auto find(const T& value) const
    requires EqualityComparable<T>
    {
        if (m_size == 0) return ConstIter{ nullptr };
        for (ConstIter beg = begin(); beg != end(); ++beg) {
            if (*beg == value) return ConstIter{ beg.m_current };
        }
        return ConstIter{ nullptr };
    }
    ~LinkedList() { clear(); }
};
}    // namespace ARLib
