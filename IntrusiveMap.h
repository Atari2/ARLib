#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "Types.h"

#ifdef DEBUG_NEW_DELETE

enum class AllocType : int { Single, Multiple };
constexpr ARLib::size_t MAP_SIZE = 400ull;

namespace ARLib {
    template <EqualityComparable Key, typename Val, size_t SIZE>
    requires Trivial<Key> && Trivial<Val>
    class IntrusiveMap;
} // namespace ARLib

using DebugNewDeleteMap = ARLib::IntrusiveMap<void*, ARLib::Pair<ARLib::size_t, AllocType>, MAP_SIZE>;

#endif

namespace ARLib {
    template <EqualityComparable Key, typename Val, size_t SIZE>
    requires Trivial<Key> && Trivial<Val>
    class IntrusiveMap {
        Key m_keys[SIZE]{};
        Val m_vals[SIZE]{};
        bool m_tombs[SIZE]{};
        size_t m_size = 0;

        static constexpr auto npos = static_cast<size_t>(-1);
        size_t search_internal(const Key& key) {
            size_t index = 0;
            for (const auto& k : m_keys) {
                if (k == key && m_tombs[index]) return index;
                index++;
            }
            return npos;
        }

        size_t search_tomb_internal() {
            size_t i = 0;
            for (auto& b : m_tombs) {
                if (!b) {
                    b = true;
                    return i;
                }
                i++;
            }
            return npos;
        }

        public:
        void insert(Key key, Val val) {
            HARD_ASSERT_FMT(m_size <= SIZE, "Intrusive map is full at size %zu", SIZE);
            auto index = search_internal(key);
            if (index == npos) {
                index = search_tomb_internal();
                if (index != npos) {
                    m_keys[index] = key;
                    m_vals[index] = val;
                    m_size++;
                }
            } else {
                m_vals[index] = val;
            }
        }
        Val remove(Key key) {
            auto index = search_internal(key);
            if (index != npos) {
                m_tombs[index] = false;
                m_size--;
                return m_vals[index];
            }
            return Val{};
        }

#ifdef DEBUG_NEW_DELETE
        ~IntrusiveMap() requires(!IsSameV<RemoveReferenceT<decltype(*this)>, DebugNewDeleteMap>) = default;
        ~IntrusiveMap() requires IsSameV<RemoveReferenceT<decltype(*this)>, DebugNewDeleteMap> {
            for (size_t i = 0; i < MAP_SIZE; i++) {
                auto [size, type] = m_vals[i];
                if (m_tombs[i]) {
                    printf("Pointer still not free at %p with size %zu and type %s\n", m_keys[i], size,
                           type == AllocType::Single ? "Single" : "Multiple");
                }
            }
        };
#endif
    };

} // namespace ARLib