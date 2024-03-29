#include <ostream>
#include <string_view>
#include "Suite.hpp"
namespace ARLib {
#ifdef COMPILER_GCC
    #if __GNUC__ >= 12
std::ostream& operator<<(std::ostream& os, const String& sv) {
    return os << std::string_view{ sv.data(), sv.size() };
}
    #endif
#else
std::ostream& operator<<(std::ostream& os, const String& sv) {
    return os << std::string_view{ sv.data(), sv.size() };
}
#endif
}    // namespace ARLib