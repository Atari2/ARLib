#pragma once
#include "Assertion.h"
#include "Concepts.h"
#include "Types.h"

namespace ARLib {
    template <EqualityComparable Key, typename Val, size_t SIZE>
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
            HARD_ASSERT(m_size <= SIZE, "Intrusive map is full");
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
    };
} // namespace ARLib