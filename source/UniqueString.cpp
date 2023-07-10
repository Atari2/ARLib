#include "UniqueString.hpp"
#include "FlatSet.hpp"
namespace ARLib {
namespace detail {
    struct UniqueStringHasher {
        Hash<String> m_hasher{};
        size_t operator()(const SharedPtr<String>& str) const noexcept { return m_hasher(*str); }
        size_t operator()(const String& str) const noexcept { return m_hasher(str); }
    };
    struct UniqueStringComparer {
        bool operator()(const SharedPtr<String>& lhs, const SharedPtr<String>& rhs) const { return lhs == rhs; }
        bool operator()(const String& lhs, const SharedPtr<String>& rhs) const { return lhs == *rhs; }
        bool operator()(const SharedPtr<String>& lhs, const String& rhs) const { return *lhs == rhs; }
    };
    using UniqueStringSet = FlatSet<SharedPtr<String>, UniqueStringHasher, UniqueStringComparer>;
    UniqueStringSet& get_interned_strings() {
        static UniqueStringSet s_interned_strings{};
        return s_interned_strings;
    }
}    // namespace detail
SharedPtr<String> UniqueString::construct(String s) {
    auto it = detail::get_interned_strings().find(s);
    if (it != detail::get_interned_strings().end()) return *it;
    SharedPtr<String> ptr{ move(s) };
    return detail::get_interned_strings().insert(move(ptr)).second();
}
UniqueString::~UniqueString() {
    if (!m_ref.exists()) return;
    auto& tbl = detail::get_interned_strings();
    auto it   = tbl.find(m_ref);
    HARD_ASSERT(it != tbl.end(), "UniqueString is not in table, this shouldn't happen");
    m_ref.reset();
    if ((*it).refcount() == 1) tbl.remove(*it);
}
}    // namespace ARLib
