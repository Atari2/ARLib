#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "Utility.h"
namespace ARLib {

    template <typename T>
    class LinkedList;

    template <typename T>
    class LinkedListIteratorBase;

    template <typename T>
    class LinkedListIterator;

    template <typename T>
    requires CopyConstructible<T> || MoveConstructible<T> || CopyAssignable<T> || MoveAssignable<T> class ListEntry {
        T m_entry;
        ListEntry<T>* m_next;
        friend class LinkedList<T>;
        friend class LinkedListIteratorBase<T>;
        friend class LinkedListIterator<T>;

        ListEntry(T&& entry, ListEntry<T>* next) requires MoveConstructible<T> : m_entry(move(entry)), m_next(next) {}

        ListEntry(const T& entry, ListEntry<T>* next) requires CopyConstructible<T> : m_entry(entry), m_next(next) {}

        T& entry__() { return m_entry; }
        ListEntry<T>* next__() const { return m_next; }

        void swap_next(ListEntry<T>* new_next) { m_next = new_next; }

        public:
        const T& entry() const { return m_entry; }
        const ListEntry<T>* next() const { return m_next; }

        ListEntry<T>& operator=(const ListEntry<T>& other) requires CopyAssignable<T> {
            if (this == &other) return *this;
            m_entry = other.m_entry;
            m_next = other.m_next;
            return *this;
        }

        ListEntry<T>& operator=(ListEntry<T>&& other)  noexcept requires MoveAssignable<T> {
            m_entry = move(other.m_entry);
            m_next = other.m_next;
            other.m_next = nullptr;
            return *this;
        }

        ~ListEntry() { m_entry.~T(); }
    };

    template <typename T>
    class LinkedListIteratorBase {
        using Entry = ListEntry<T>;
        friend LinkedList<T>;

        protected:
        Entry* m_current;
        virtual void internal_advance_() = 0;

        public:
        using Type = T;
        LinkedListIteratorBase(Entry* current) : m_current(current) {}

        const Entry* current() const { return m_current; }
        bool is_done() const { return m_current->next() == nullptr; }
        bool operator==(const LinkedListIteratorBase<T>& other) const { return m_current == other.m_current; }
        bool operator!=(const LinkedListIteratorBase<T>& other) const { return m_current != other.m_current; }

        T& operator*() const { return m_current->entry__(); }
        virtual ~LinkedListIteratorBase() = default;
    };

#define m_current LinkedListIteratorBase<T>::m_current
    template <typename T>
    class LinkedListIterator final : public LinkedListIteratorBase<T> {
        using Entry = ListEntry<T>;
        virtual void internal_advance_() override {
            HARD_ASSERT(m_current, "m_current must not be nullptr")
            m_current = m_current->next__();
        }

        public:
        LinkedListIterator(Entry* current) : LinkedListIteratorBase<T>(current) {}
        LinkedListIterator<T>& operator=(const LinkedListIterator<T>& other) { m_current = other.m_current; }
        LinkedListIterator<T>& operator=(LinkedListIterator<T>&& other)  noexcept {
            m_current = other.m_current;
            other.m_current = nullptr;
        }
        LinkedListIterator<T>& operator++() {
            internal_advance_();
            return *this;
        }

        LinkedListIterator<T> operator++(int) {
            LinkedListIterator<T> prev{m_current};
            internal_advance_();
            return prev;
        }
        virtual ~LinkedListIterator() = default;
    };
#undef m_current
    // LIFO
    template <typename T>
    class LinkedList {
        public:
        using Entry = ListEntry<T>;
        using Iter = LinkedListIterator<T>;

        private:
        Entry* m_head = nullptr;
        size_t m_size = 0;

        void internal_single_prepend_(T&& value) {
            m_head = new Entry(Forward<T>(value), m_head);
            m_size++;
        }

        template <typename... Args>
        void internal_prepend_(T&& value, Args&&... values) {
            if constexpr (sizeof...(values) == 0) {
                internal_single_prepend_(Forward<T>(value));
            } else {
                internal_single_prepend_(Forward<T>(value));
                internal_prepend_(Forward<Args>(values)...);
            }
        }

        public:
        template <typename... Args>
        LinkedList(Args&&... values) {
            internal_prepend_(Forward<Args>(values)...);
        }

        LinkedList(std::initializer_list<T> values) {
            for (auto v : values)
                internal_single_prepend_(Forward<T>(v));
        }

        LinkedList() = default;

        LinkedList(const LinkedList<T>& other) : m_head(other.m_head), m_size(other.m_size) {}

        LinkedList(LinkedList<T>&& other) noexcept : m_head(other.m_head), m_size(other.m_size) {
            other.m_head = nullptr;
            other.m_size = 0;
        }

        LinkedList<T>& operator=(LinkedList<T>&& other) noexcept {
            m_head = other.m_head;
            m_size = other.m_size;
            other.m_head = nullptr;
            other.m_size = 0;
            return *this;
        }

        
        void prepend(const T& value) { internal_single_prepend_(Forward<T>(T{value})); }

        void append(const T& value) {
            auto* curr = m_head;
            while (curr->next() != nullptr) {
                curr = curr->next__();
            }
            curr->swap_next(new Entry(value, nullptr));
            m_size++;
        }

        void prepend(T&& value) { internal_single_prepend_(Forward<T>(value)); }

        void append(T&& value) {
            auto* curr = m_head;
            while (curr->next() != nullptr) {
                curr = curr->next__();
            }
            curr->swap_next(new Entry(value, nullptr));
            m_size++;
        }

        void pop_head() {
            if (!m_head) return;
            auto* rem = m_head;
            m_head = m_head->next__();
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
            while (curr->next()->next() != nullptr) {
                curr = curr->next();
            }
            delete curr->next();
            curr->swap_next(nullptr);
            m_size--;
        }

        void remove(const Entry* entry) {
            if (!m_head) return;
            if (entry == m_head) {
                auto* del = m_head;
                m_head = m_head->next__();
                m_size--;
                delete del;
            } else {
                auto* prev = m_head;
                auto* del = prev->next__();
                while (del) {
                    if (del == entry) {
                        prev->swap_next(del->next__());
                        delete del;
                        m_size--;
                        break;
                    }
                    prev = del;
                    del = prev->next__();
                }
            }
        }

        void remove(const T& item) {
            if (!m_head) return;
            Iter iter = find(item);
            if (iter == end()) return;
            const Entry* entry = iter.current();
            remove(entry);
        }

        const T& peek() const {
            if (m_size == 0) return {};
            return m_head->entry();
        }

        void clear() {
            while (m_size > 0)
                pop_head();
        }

        Iter begin() const { return {m_head}; }
        Iter end() const { return {nullptr}; }

        size_t size() const { return m_size; }

        template <typename Functor>
        void for_each(Functor&& func) {
            if (!m_head) return;
            Entry* curr = m_head;
            while (curr) {
                func(curr->entry());
                curr = curr->next__();
            }
        }

        bool exists(const T& value) const requires EqualityComparable<T> {
            if (m_size == 0) return false;
            for (const auto& val : *this)
                if (val == value) return true;
            return false;
        }

        Iter find(const T& value) const requires EqualityComparable<T> {
            if (m_size == 0) return {nullptr};
            for (Iter beg = begin(); beg != end(); ++beg) {
                if (*beg == value) return {beg.m_current};
            }
            return {nullptr};
        }

        ~LinkedList() { clear(); }
    };
} // namespace ARLib
using ARLib::LinkedList;