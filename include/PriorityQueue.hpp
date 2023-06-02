#pragma once
#include "Allocator.hpp"
#include "Concepts.hpp"
#include "Functional.hpp"
#include "Ordering.hpp"
#include "PrintInfo.hpp"
#include "SortedVector.hpp"
#include "Types.hpp"
namespace ARLib {
template <typename Container, typename T>
concept CanAskForItem = requires(const Container& t) {
                            { t.item() } -> SameAs<const T&>;
                        } && requires(Container& t) {
                                 { t.item() } -> SameAs<T&>;
                             };

#define ENABLE_QUERY_ITEM(varName)                                                                                     \
    const auto& item() const { return varName; }                                                                       \
    auto& item() { return varName; }

enum class PriorityQueueVisitingOrder { Ascending, Descending };
template <typename T, typename CustomQueueItem = VoidT<>>
class PriorityQueue {
    friend PrintInfo<PriorityQueue<T, CustomQueueItem>>;

    public:
    struct DefaultQueueItem {
        size_t priority{ 0 };
        T _item;
        Ordering operator<=>(const DefaultQueueItem& other) const {
            if (priority > other.priority)
                return greater;
            else if (priority < other.priority)
                return less;
            return equal;
        }
        T& item() { return _item; }
        const T& item() const { return _item; }
    };
    using QueueItem = ConditionalT<IsVoid<CustomQueueItem>::value, DefaultQueueItem, CustomQueueItem>;

    private:
    constexpr static inline bool UsingDefault = IsSameV<QueueItem, DefaultQueueItem>;
    constexpr static inline bool CanGiveItem  = CanAskForItem<QueueItem, T>;
    static inline auto s_default_function     = [](const QueueItem& left, const QueueItem& right) -> Ordering {
        if constexpr (UsingDefault) { return left <=> right; }
        ASSERT_NOT_REACHED("Invalid queue item");
    };

    using PriorityFunc = Function<Ordering(const QueueItem&, const QueueItem&)>;
    struct PriorityFuncWrap {
        mutable PriorityFunc func{ s_default_function };
        arlib_forceinline Ordering operator()(const QueueItem& left, const QueueItem& right) const {
            return func(left, right);
        }
    };
    SortedVector<QueueItem, PriorityFuncWrap> m_queue;
    using RetType = ConditionalT<UsingDefault || CanGiveItem, T, CustomQueueItem>;

    public:
    PriorityQueue()
    requires Orderable<QueueItem>
    = default;
    PriorityQueue(PriorityFunc&& function) { m_queue.ordering().func = move(function); }
    void push(T&& item, size_t priority)
    requires UsingDefault
    {
        m_queue.insert(QueueItem{ priority, move(item) });
    }
    void push(QueueItem item) { m_queue.insert(move(item)); }
    RetType pop() {
        HARD_ASSERT(m_queue.size() > 0, "Trying to pop from empty queue");
        if constexpr (CanGiveItem) {
            RetType val = move(m_queue.pop().item());
            return val;
        } else {
            RetType val = move(m_queue.pop());
            return val;
        }
    }
    RetType pop_lowest() {
        HARD_ASSERT(m_queue.size() > 0, "Trying to pop from empty queue");
        if constexpr (CanGiveItem) {
            RetType val = move(m_queue[0].item());
            m_queue.remove(0);
            return val;
        } else {
            RetType val = move(m_queue[0]);
            m_queue.remove(0);
            return val;
        }
    }
    const RetType& operator[](size_t idx) const {
        if constexpr (CanGiveItem)
            return m_queue[idx].item();
        else
            return m_queue[idx];
    }
    RetType& operator[](size_t idx) {
        if constexpr (CanGiveItem)
            return m_queue[idx].item();
        else
            return m_queue[idx];
    }
    size_t size() const { return m_queue.size(); }
    template <typename Functor>
    requires CallableWith<Functor, RetType>
    void for_each(Functor&& func, PriorityQueueVisitingOrder order = PriorityQueueVisitingOrder::Descending) const {
        if constexpr (CanGiveItem) {
            if (order == PriorityQueueVisitingOrder::Descending) {
                for (size_t i = m_queue.size() - 1;; i--) {
                    func(m_queue[i].item());
                    if (i == 0) break;
                }
            } else {
                for (auto& q : m_queue) func(q.item());
            }
        } else {
            if (order == PriorityQueueVisitingOrder::Descending) {
                for (size_t i = m_queue.size() - 1;; i--) {
                    func(m_queue[i]);
                    if (i == 0) break;
                }
            } else {
                for (auto& q : m_queue) func(q);
            }
        }
    }
};
template <Printable T>
struct PrintInfo<PriorityQueue<T, VoidT<>>> {
    const PriorityQueue<T, VoidT<>>& m_queue;
    PrintInfo(const PriorityQueue<T, VoidT<>>& queue) : m_queue(queue) {}
    String repr() const {
        String val{ "[ " };
        for (size_t i = m_queue.size() - 1;; i--) {
            String g = PrintInfo<T>{ m_queue.m_queue[i].item() }.repr();
            val += String::formatted("{.item: %s, .priority: %u}", g.data(), m_queue.m_queue[i].priority);
            val += ", "_s;
            if (i == 0) break;
        }
        return val.substring(0, val.size() - 2) + " ]"_s;
    }
};
// clang-format off
    template <typename T, typename CustomQueueItem>
    requires(!SameAs<CustomQueueItem, VoidT<>> &&       // CustomQueueItem is not void and
             ((CanAskForItem<CustomQueueItem, T> && Printable<T>) ||    // can query the container for the item and the item is printable
              Printable<CustomQueueItem>))                  // or otherwise the container itself is printable
    struct PrintInfo<PriorityQueue<T, CustomQueueItem>> {
        const PriorityQueue<T, CustomQueueItem>& m_queue;
        PrintInfo(const PriorityQueue<T, CustomQueueItem>& queue) : m_queue(queue) {}
        String repr() const {
            String val{"[ "};
            for (size_t i = m_queue.size() - 1;; i--) {
                if constexpr (CanAskForItem<CustomQueueItem, T> && Printable<T>)
                    val += PrintInfo<CustomQueueItem>{m_queue[i].item()}.repr();
                else
                    val += PrintInfo<CustomQueueItem>{m_queue[i]}.repr();
                val += ", "_s;
                if (i == 0) break;
            }
            return val.substring(0, val.size() - 2) + " ]"_s;
        }
    };
// clang-format on
}    // namespace ARLib
