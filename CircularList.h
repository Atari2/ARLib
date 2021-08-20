#pragma once
#include "Concepts.h"
#include "Utility.h"

namespace ARLib {
    template <typename T>
    class CircularDoublyLinkedList;

    template <typename T>
    class CircularListIteratorBase;

    template <typename T>
    class CircularListIterator;

    template <typename T>
    class ReverseCircularListIterator;

    template <typename T>
    requires CopyConstructible<T> || MoveConstructible<T> || CopyAssignable<T> ||
    MoveAssignable<T> class CircularListEntry {
        friend class CircularDoublyLinkedList<T>;
        friend class CircularListIteratorBase<T>;
        friend class CircularListIterator<T>;
        friend class ReverseCircularListIterator<T>;

        T m_entry;
        CircularListEntry<T>* m_prev = nullptr;
        CircularListEntry<T>* m_next = nullptr;

        CircularListEntry(T&& entry, CircularListEntry<T>* prev,
                          CircularListEntry<T>* next) requires MoveConstructible<T> :
            m_entry(move(entry)),
            m_prev(prev),
            m_next(next) {
            if (prev) prev->m_next = this;
            if (next) next->m_prev = this;
        }

        CircularListEntry(const T& entry, CircularListEntry<T>* prev,
                          CircularListEntry<T>* next) requires CopyConstructible<T> :
            m_entry(entry),
            m_prev(prev),
            m_next(next) {
            if (prev) prev->m_next = this;
            if (next) next->m_prev = this;
        }

        T& entry__() { return m_entry; }
        CircularListEntry<T>* prev__() const { return m_prev; }
        CircularListEntry<T>* next__() const { return m_next; }

        void swap_prev(CircularListEntry<T>* new_prev) { m_prev = new_prev; }
        void swap_next(CircularListEntry<T>* new_next) { m_next = new_next; }
        void swap_both(CircularListEntry<T>* new_prev, CircularListEntry<T>* new_next) {
            m_next = new_next;
            m_prev = new_prev;
        }

        public:
        const T& entry() const { return m_entry; }
        const CircularListEntry<T>* prev() const { return m_prev; }
        const CircularListEntry<T>* next() const { return m_next; }

        CircularListEntry<T>& operator=(const CircularListEntry<T>& other) requires CopyAssignable<T> {
            if (this == &other) return *this;
            m_entry = other.m_entry;
            m_prev = other.m_prev;
            m_next = other.m_next;
            return *this;
        }

        CircularListEntry<T>& operator=(CircularListEntry<T>&& other)  noexcept requires MoveAssignable<T> {
            m_entry = move(other.m_entry);
            m_prev = other.m_prev;
            m_next = other.m_next;
            other.m_prev = nullptr;
            other.m_next = nullptr;
            return *this;
        }

        ~CircularListEntry() { m_entry.~T(); }
    };

    template <typename T>
    class CircularListIteratorBase {
        using Entry = CircularListEntry<T>;

        protected:
        const Entry* m_current;
        const Entry* m_begin;
        bool at_end = false;
        virtual void internal_advance_() = 0;
        virtual void internal_revert_() = 0;

        public:
        using Type = T;
        explicit CircularListIteratorBase(const Entry* current) : m_current(current), m_begin(current) {}
        CircularListIteratorBase(const Entry* current, bool end) : m_current(current), m_begin(current), at_end(end) {}

        const Entry* current() const { return m_current; }

        bool is_done() { return at_end; }

        bool operator==(const CircularListIteratorBase<T>& other) const {
            if (at_end == other.at_end && m_current == other.m_current) return true;
            return false;
        }
        bool operator!=(const CircularListIteratorBase<T>& other) const {
            if (m_current != other.m_current) return true;
            return !at_end;
        }

        const T& operator*() const { return m_current->entry(); }
    };
#define m_current CircularListIteratorBase<T>::m_current
#define m_begin   CircularListIteratorBase<T>::m_begin
#define at_end    CircularListIteratorBase<T>::at_end
    template <typename T>
    class CircularListIterator final : public CircularListIteratorBase<T> {
        using Entry = CircularListEntry<T>;
        void internal_advance_() override {
            if (at_end) return;
            m_current = m_current->next();
            if (m_current == m_begin) { at_end = true; }
        }

        void internal_revert_() override {
            if (at_end) return;
            m_current = m_current->prev();
            if (m_current == m_begin) { at_end = true; }
        }

        public:
        explicit CircularListIterator(const Entry* current) : CircularListIteratorBase<T>(current) {}
        CircularListIterator(const Entry* current, bool end) : CircularListIteratorBase<T>(current, end) {}

        CircularListIterator<T>& operator=(const CircularListIterator<T>& other) { m_current = other.m_current; return *this; }

        CircularListIterator<T>& operator=(CircularListIterator<T>&& other)  noexcept {
            m_current = other.m_current;
            other.m_current = nullptr;
        }

        CircularListIterator<T>& operator++() {
            internal_advance_();
            return *this;
        }

        CircularListIterator<T> operator++(int) {
            CircularListIterator<T> prev{m_current};
            internal_advance_();
            return prev;
        }

        CircularListIterator<T>& operator--() {
            internal_revert_();
            return *this;
        }

        CircularListIterator<T> operator--(int) {
            CircularListIterator<T> prev{m_current};
            internal_revert_();
            return prev;
        }

        CircularListIterator<T> operator-(int offset) {
            CircularListIterator<T> iter = *this;
            for (int i = 0; i < offset; i++)
                iter.internal_revert_();
            return iter;
        }
    };

    template <typename T>
    class ReverseCircularListIterator final : public CircularListIteratorBase<T> {
        using Entry = CircularListEntry<T>;
        void internal_advance_() override {
            if (at_end) return;
            m_current = m_current->prev();
            if (m_current == m_begin) { at_end = true; }
        }

        void internal_revert_() override {
            if (at_end) return;
            m_current = m_current->next();
            if (m_current == m_begin) { at_end = true; }
        }

        public:
        explicit ReverseCircularListIterator(const Entry* current) : CircularListIteratorBase<T>(current) {}
        ReverseCircularListIterator(const Entry* current, bool end) : CircularListIteratorBase<T>(current, end) {}

        ReverseCircularListIterator<T>& operator=(const ReverseCircularListIterator<T>& other) {
            m_current = other.m_current;
        }

        ReverseCircularListIterator<T>& operator=(ReverseCircularListIterator<T>&& other) noexcept {
            m_current = other.m_current;
            other.m_current = nullptr;
            return *this;
        }

        ReverseCircularListIterator<T>& operator++() {
            internal_advance_();
            return *this;
        }

        ReverseCircularListIterator<T> operator++(int) {
            ReverseCircularListIterator<T> prev{m_current};
            internal_advance_();
            return prev;
        }

        ReverseCircularListIterator<T>& operator--() {
            internal_revert_();
            return *this;
        }

        ReverseCircularListIterator<T> operator--(int) {
            ReverseCircularListIterator<T> prev{m_current};
            internal_revert_();
            return prev;
        }

        ReverseCircularListIterator<T> operator-(int offset) {
            ReverseCircularListIterator<T> iter = *this;
            for (int i = 0; i < offset; i++)
                iter.internal_revert_();
            return iter;
        }
    };

#undef m_current
#undef m_begin
#undef at_end

    template <typename T>
    class CircularDoublyLinkedList {
        public:
        using Entry = CircularListEntry<T>;
        using Iter = CircularListIterator<T>;
        using ReverseIter = ReverseCircularListIterator<T>;

        private:
        Entry* m_first = nullptr;
        Entry* m_last = nullptr;
        size_t m_size = 0;

        void internal_single_append_(T&& value) {
            if (m_size == 0) {
                auto* val = new Entry(Forward<T>(value), nullptr, nullptr);
                val->swap_both(val, val);
                m_first = val;
                m_last = val;
            } else {
                Entry* last = m_last;
                m_last = new Entry(Forward<T>(value), last, m_first);
            }
            m_size++;
        }

        template <typename... Args>
        void internal_append_(T&& value, Args&&... values) {
            if constexpr (sizeof...(values) == 0) {
                internal_single_append_(Forward<T>(value));
            } else {
                internal_single_append_(Forward<T>(value));
                internal_append_(Forward<Args>(values)...);
            }
        }

        public:
        template <typename... Args>
        explicit CircularDoublyLinkedList(Args&&... values) {
            internal_append_(Forward<Args>(values)...);
        }

        CircularDoublyLinkedList(std::initializer_list<T> values) {
            for (auto v : values)
                internal_single_append_(Forward<T>(v));
        }

        CircularDoublyLinkedList() = default;

        CircularDoublyLinkedList(const CircularDoublyLinkedList<T>& other) :
            m_first(other.m_first), m_last(other.m_last), m_size(other.m_size) {}

        CircularDoublyLinkedList(CircularDoublyLinkedList<T>&& other)  noexcept :
            m_first(other.m_first), m_last(other.m_last), m_size(other.m_size) {
            other.m_first = nullptr;
            other.m_last = nullptr;
            other.m_size = 0;
        }

        void append(T&& value) { internal_single_append_(Forward<T>(value)); }

        void prepend(T&& value) {
            if (m_size == 0) {
                auto* val = new Entry(value, nullptr, nullptr);
                val->swap_both(val, val);
                m_first = val;
                m_last = val;
            } else {
                Entry* first = m_first;
                m_first = new Entry(value, m_last, first);
            }
            m_size++;
        }

        void pop() {
            if (m_size == 0) return;
            Entry* ret = m_last;
            m_last = m_last->prev__();
            m_first->swap_prev(m_last);
            delete ret;
            m_size--;
        }

        void pop_head() {
            if (m_size == 0) return;
            Entry* ret = m_first;
            m_first = m_first->next__();
            m_last->swap_next(m_first);
            delete ret;
            m_size--;
        }

        void remove(const Entry* entry) {
            if (m_size == 0) return;
            if (entry == m_first) {
                m_first = entry->next__();
            } else if (entry == m_last) {
                m_last = entry->prev__();
            }
            entry->prev__()->swap_next(entry->next__());
            entry->next__()->swap_prev(entry->prev__());
            m_size--;
            delete entry;
        }

        void remove(const T& item) {
            if (m_size == 0) return;
            Iter iter = find(item);
            if (iter == end()) return;
            const Entry* entry = iter.current();
            remove(entry);
        }

        const T& peek() const {
            if (m_size == 0) return {};
            return m_last->entry();
        }
        const T& peek_head() const {
            if (m_size == 0) return {};
            return m_first->entry();
        }

        void clear() {
            while (m_size > 0)
                pop();
            m_first = nullptr;
            m_last = nullptr;
        }

        Iter begin() const {
            if (m_size == 0) return {m_first, true};
            return {m_first};
        }
        Iter end() const { return {m_first, true}; }

        ReverseIter rbegin() const {
            if (m_size == 0) return {m_last, true};
            return {m_last};
        }
        ReverseIter rend() const { return {m_last, true}; }

        size_t size() const { return m_size; }

        template <typename Functor>
        void for_each(Functor&& func) {
            if (m_size == 0) return;
            Entry* curr = m_first;
            for (;;) {
                func(curr->entry());
                if (curr == m_last) break;
                curr = curr->next__();
            }
        }

        bool exists(const T& value) const {
            if (m_size == 0) return false;
            for (const auto& val : *this) {
                if (val == value) return true;
            }
            return false;
        }

        Iter find(const T& value) const {
            if (m_size == 0) return {m_last, true};
            for (Iter beg = begin(); beg != end(); ++beg) {
                if (*beg == value) return {beg.current()};
            }
            return {m_first, true};
        }

        ReverseIter rfind(const T& value) const {
            if (m_size == 0) return {m_last, true};
            for (ReverseIter beg = rbegin(); beg != rend(); ++beg) {
                if (*beg == value) return {beg.current()};
            }
            return {m_last, true};
        }

        ~CircularDoublyLinkedList() {
            bool done = false;
            const Entry* curr = m_first;
            while (!done && curr) {
                if (curr == m_last) { done = true; }
                const Entry* next = curr->next();
                delete curr;
                curr = next;
            }
        }
    };
} // namespace ARLib
