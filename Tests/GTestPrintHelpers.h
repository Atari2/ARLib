#pragma once
#include "Suite.h"
#include <ostream>
namespace ARLib {
std::ostream& operator<<(std::ostream& os, const String& s);
template <Printable T>
std::ostream& operator<<(std::ostream& os, const T& obj) {
    return os << PrintInfo<T>{ obj }.repr();
}
}    // namespace ARLib