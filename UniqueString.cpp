#include "UniqueString.h"

namespace ARLib {
    namespace detail {
        static LinkedSet<String>& get_interned_strings() {
            static LinkedSet<String> s_interned_strings{};
            return s_interned_strings;
        }
    } // namespace detail

    String* UniqueString::construct(const String& s) { return &ARLib::detail::get_interned_strings().prepend(s); }
} // namespace ARLib