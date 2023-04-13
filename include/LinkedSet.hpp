#pragma once
#include "Assertion.hpp"
#include "Concepts.hpp"
#include "Utility.hpp"
namespace ARLib {
template <typename T>
class LinkedSet {
    struct SetEntry {
        T m_entry;
        SetEntry* m_next;
        SetEntry(T&& entry, SetEntry* next) : m_entry(move(entry)), m_next(next) {}
        SetEntry(const T& entry, SetEntry* next) : m_entry(entry), m_next(next) {}
        T& entry_mut() { return m_entry; }
        SetEntry* next_mut() const { return m_next; }
        void swap_next(SetEntry* new_next) { m_next = new_next; }

        public:
        const T& entry() const { return m_entry; }
        const SetEntry* next() const { return m_next; }
        SetEntry& operator=(const SetEntry& other)
        requires CopyAssignable<T>
        {
            if (this == &other) return *this;
            m_entry = other.m_entry;
            m_next  = other.m_next;
            return *this;
        }
        SetEntry& operator=(SetEntry&& other) noexcept
        requires MoveAssignable<T>
        {
            m_entry      = move(other.m_entry);
            m_next       = other.m_next;
            other.m_next = nullptr;
            return *this;
        }
        ~SetEntry() { m_entry.~T(); }
    };
    class LinkedSetIterator {
        friend LinkedSet<T>;
        SetEntry* m_current;
        void internal_advance_() {
            HARD_ASSERT(m_current, "m_current must not be nullptr")
            m_current = m_current->next_mut();
        }

        public:
        explicit LinkedSetIterator(SetEntry* current) : m_current(current) {}
        LinkedSetIterator& operator=(const LinkedSetIterator& other) {
            if (this == &other) return *this;
            m_current = other.m_current;
            return *this;
        }
        LinkedSetIterator& operator=(LinkedSetIterator&& other) noexcept {
            m_current       = other.m_current;
            other.m_current = nullptr;
            return *this;
        }
        const SetEntry* current() const { return m_current; }
        bool is_done() const { return m_current->next() == nullptr; }
        bool operator==(const LinkedSetIterator& other) const { return m_current == other.m_current; }
        bool operator!=(const LinkedSetIterator& other) const { return m_current != other.m_current; }
        T& operator*() const { return m_current->entry_mut(); }
        LinkedSetIterator& operator++() {
            internal_advance_();
            return *this;
        }
        LinkedSetIterator operator++(int) {
            LinkedSetIterator prev{ m_current };
            internal_advance_();
            return prev;
        }
    };
    SetEntry* m_head = nullptr;
    size_t m_size    = 0;
    T& internal_single_prepend_(T&& value) {
        auto iter = find(value);
        if (iter != end()) { return *iter; }
        m_head = new SetEntry(Forward<T>(value), m_head);
        m_size++;
        return m_head->entry_mut();
    }

    public:
    template <typename... Args>
    explicit LinkedSet(T&& val, Args&&... values)
    requires AllOfV<T, Args...>
    {
        internal_single_prepend_(Forward<T>(val));
        (internal_single_prepend_(Forward<Args>(values)), ...);
    }
    LinkedSet() = default;
    LinkedSet(const LinkedSet& other) : m_head(other.m_head), m_size(other.m_size) {}
    LinkedSet(LinkedSet&& other) noexcept : m_head(other.m_head), m_size(other.m_size) {
        other.m_head = nullptr;
        other.m_size = 0;
    }
    LinkedSet& operator=(LinkedSet&& other) noexcept {
        m_head       = other.m_head;
        m_size       = other.m_size;
        other.m_head = nullptr;
        other.m_size = 0;
        return *this;
    }
    T& prepend(const T& value) { return internal_single_prepend_(Forward<T>(T{ value })); }
    T& append(const T& value) {
        auto& iter = find(value);
        if (iter != end()) return *iter;
        auto* curr = m_head;
        while (curr->next() != nullptr) { curr = curr->next_mut(); }
        curr->swap_next(new SetEntry(value, nullptr));
        m_size++;
        return curr->next_mut()->entry_mut();
    }
    T& prepend(T&& value) { return internal_single_prepend_(Forward<T>(value)); }
    T& append(T&& value) {
        auto iter = find(value);
        if (iter != end()) return *iter;
        auto* curr = m_head;
        while (curr->next() != nullptr) { curr = curr->next_mut(); }
        curr->swap_next(new SetEntry(value, nullptr));
        m_size++;
        return curr->next_mut()->entry_mut();
    }
    void pop_head() {
        if (!m_head) return;
        auto* rem = m_head;
        m_head    = m_head->next_mut();
        m_size--;
        delete rem;
    }
    void pop() {
        auto* curr = m_head;
        if (!curr) return;
        if (curr->next() == nullptr) {
            m_head = nullptr;
            delete curr;
            return;
        }
        while (curr->next()->next() != nullptr) { curr = curr->next(); }
        delete curr->next();
        curr->swap_next(nullptr);
        m_size--;
    }
    void remove(const SetEntry* entry) {
        if (!m_head) return;
        if (entry == m_head) {
            auto* del = m_head;
            m_head    = m_head->next_mut();
            m_size--;
            delete del;
        } else {
            auto* prev = m_head;
            auto* del  = prev->next_mut();
            while (del) {
                if (del == entry) {
                    prev->swap_next(del->next_mut());
                    delete del;
                    m_size--;
                    break;
                }
                prev = del;
                del  = prev->next_mut();
            }
        }
    }
    void remove(const T& item) {
        if (!m_head) return;
        LinkedSetIterator iter = find(item);
        if (iter == end()) return;
        const SetEntry* entry = iter.current();
        remove(entry);
    }
    const T& peek() const
    requires DefaultConstructible<T>
    {
        if (m_size == 0) return {};
        return m_head->entry();
    }
    void clear() {
        while (m_size > 0) pop_head();
    }
    LinkedSetIterator begin() const { return LinkedSetIterator{ m_head }; }
    LinkedSetIterator end() const { return LinkedSetIterator{ nullptr }; }
    size_t size() const { return m_size; }
    template <typename Functor>
    void for_each(Functor&& func) {
        if (!m_head) return;
        SetEntry* curr = m_head;
        while (curr) {
            func(curr->entry());
            curr = curr->next_mut();
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
    LinkedSetIterator find(const T& value) const
    requires EqualityComparable<T>
    {
        if (m_size == 0) return LinkedSetIterator{ nullptr };
        for (LinkedSetIterator beg = begin(); beg != end(); ++beg) {
            if (*beg == value) return LinkedSetIterator{ beg.m_current };
        }
        return LinkedSetIterator{ nullptr };
    }
    ~LinkedSet() { clear(); }
};
}    // namespace ARLib
