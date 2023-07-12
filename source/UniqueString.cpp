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
SharedPtr<String> UniqueString::construct(const String& s) {
    auto it = detail::get_interned_strings().find(s);
    if (it != detail::get_interned_strings().end()) return *it;
    return detail::get_interned_strings().insert(SharedPtr{ String{ s } }).second();
}
}    // namespace ARLib
