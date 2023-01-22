#include "UniqueString.h"
#include "Set.h"
namespace ARLib {
namespace detail {
    struct UniqueStringSetComparator {
        static bool equal(const SharedPtr<String>& first, const SharedPtr<String>& second) { return *first == *second; }
    };
    using UniqueStringSet = Set<SharedPtr<String>, UniqueStringSetComparator>;
    UniqueStringSet& get_interned_strings() {
        static UniqueStringSet s_interned_strings{};
        return s_interned_strings;
    }
}    // namespace detail
SharedPtr<String> UniqueString::construct(String s) {
    SharedPtr<String> ptr{ move(s) };
    return detail::get_interned_strings().insert(ptr);
}
UniqueString::~UniqueString() {
    auto& tbl = detail::get_interned_strings();
    auto it   = tbl.find(m_ref);
    HARD_ASSERT(it != tbl.end(), "UniqueString is not in table, this shouldn't happen");
    m_ref.reset();
    if ((*it).refcount() == 1) tbl.remove(*it);
}
}    // namespace ARLib
