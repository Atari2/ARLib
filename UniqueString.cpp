#include "UniqueString.h"
#include "LinkedSet.h"

namespace ARLib {
    namespace detail {
        static LinkedSet<String>& get_interned_strings() {
            static LinkedSet<String> s_interned_strings{};
            return s_interned_strings;
        }
    } // namespace detail

    WeakPtr<String> UniqueString::construct(String s) {
        return WeakPtr{ARLib::detail::get_interned_strings().prepend(Forward<String>(s))};
    }
} // namespace ARLib