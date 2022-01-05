#pragma once
#include "Allocator.h"
#include "Concepts.h"
#include "Functional.h"
#include "Ordering.h"
#include "PrintInfo.h"
#include "SortedVector.h"
#include "Types.h"

namespace ARLib {

    template <typename T, typename CustomQueueItem = VoidT<>>
    class PriorityQueue {

        friend PrintInfo<PriorityQueue<T, CustomQueueItem>>;

        public:
        struct DefaultQueueItem {
            size_t priority{0};
            T item;
            Ordering operator<=>(const DefaultQueueItem& other) const {
                if (priority > other.priority)
                    return greater;
                else if (priority < other.priority)
                    return less;
                return equal;
            }
        };

        using QueueItem = ConditionalT<IsVoid<CustomQueueItem>::value, DefaultQueueItem, CustomQueueItem>;

        private:
        static inline auto s_default_function = [](const QueueItem& left, const QueueItem& right) -> Ordering {
            return left <=> right;
        };

        using PriorityFunc = Function<Ordering(const QueueItem&, const QueueItem&)>;
        struct PriorityFuncWrap {
            mutable PriorityFunc func{s_default_function};
            forceinline Ordering operator()(const QueueItem& left, const QueueItem& right) const {
                return func(left, right);
            }
        };
        SortedVector<QueueItem, PriorityFuncWrap> m_queue;

        constexpr static inline bool UsingDefault = IsSameV<QueueItem, DefaultQueueItem>;

        public:
        PriorityQueue() requires Orderable<QueueItem>
        = default;
        PriorityQueue(PriorityFunc&& function) { m_queue.ordering().func = move(function); }
        void push(T&& item, size_t priority) requires UsingDefault { m_queue.insert(QueueItem{priority, move(item)}); }
        void push(QueueItem item) { m_queue.insert(move(item)); }
        auto pop() {
            HARD_ASSERT(m_queue.size() > 0, "Trying to pop from empty queue");
            if constexpr (UsingDefault) {
                T val = move(m_queue.pop().item);
                return val;
            } else {
                T val = move(m_queue.pop());
                return val;
            }
        }
        auto pop_lowest() {
            HARD_ASSERT(m_queue.size() > 0, "Trying to pop from empty queue");
            if constexpr (UsingDefault) {
                T val = move(m_queue[0].item);
                m_queue.remove(0);
                return val;
            } else {
                T val = move(m_queue[0]);
                m_queue.remove(0);
                return val;
            }
        }
        const auto& operator[](size_t idx) const {
            if constexpr (UsingDefault)
                return m_queue[idx].item;
            else
                return m_queue[idx];
        }
        auto& operator[](size_t idx) {
            if constexpr (UsingDefault)
                return m_queue[idx].item;
            else
                return m_queue[idx];
        }
        size_t size() const { return m_queue.size(); }
    };

    template <Printable T>
    struct PrintInfo<PriorityQueue<T, VoidT<>>> {
        const PriorityQueue<T, VoidT<>>& m_queue;
        PrintInfo(const PriorityQueue<T, VoidT<>>& queue) : m_queue(queue) {}
        String repr() const {
            String val{"[ "};
            for (size_t i = m_queue.size() - 1;; i--) {
                String g = PrintInfo<T>{m_queue.m_queue[i].item}.repr();
                val += String::formatted("{.item: %s, .priority: %u}", g.data(), m_queue.m_queue[i].priority);
                val += ", "_s;
                if (i == 0) break;
            }
            return val.substring(0, val.size() - 2) + " ]"_s;
        }
    };
    template <Printable T, typename CustomQueueItem>
    requires(!SameAs<CustomQueueItem, VoidT<>>)
    struct PrintInfo<PriorityQueue<T, CustomQueueItem>>  {
        const PriorityQueue<T, CustomQueueItem>& m_queue;
        PrintInfo(const PriorityQueue<T, CustomQueueItem>& queue) : m_queue(queue) {}
        String repr() const {
            String val{"[ "};
            for (size_t i = m_queue.size() - 1;; i--) {
                val += PrintInfo<T>{m_queue[i]}.repr();
                val += ", "_s;
                if (i == 0) break;
            }
            return val.substring(0, val.size() - 2) + " ]"_s;
        }
    };
} // namespace ARLib