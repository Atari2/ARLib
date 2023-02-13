#include <ostream>
#include <string_view>
#include "Suite.h"
namespace ARLib {
std::ostream& operator<<(std::ostream& os, const String& sv) {
    return os << std::string_view{ sv.data(), sv.size() };
}
}    // namespace ARLib