#pragma once
#include "Suite.h"
#include <ostream>
namespace ARLib {
#ifdef COMPILER_GCC
    #if __GNUC__ >= 12
std::ostream& operator<<(std::ostream& os, const String& s);
template <Printable T>
std::ostream& operator<<(std::ostream& os, const T& obj) {
    return os << PrintInfo<T>{ obj }.repr();
}
    #endif
#endif
}    // namespace ARLib