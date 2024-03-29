#pragma once
#include <typeinfo>
namespace ARLib {
using TypeInfo = std::type_info;
class DemangledInfo {
    const char* m_demangled_name  = nullptr;
    mutable bool m_should_dealloc = true;

    public:
    DemangledInfo(const TypeInfo& info, bool critical = false);
    DemangledInfo(const char* mangled_name, bool critical = true);
    // leaks the memory associated with the buffer
    // you'll have to manual free this one if used.
    const char* leak_name() const {
        m_should_dealloc = false;
        return m_demangled_name;
    }
    const char* name() const { return m_demangled_name; }
    ~DemangledInfo();
};
template <typename T>
auto demangled_info_for() {
    return DemangledInfo{ typeid(T) };
}
}    // namespace ARLib
#define MANGLED_TYPENAME_TO_STRING(type) typeid(type).name()

// On Linux, this macro will leak memory if called. Usually better to use it when you know that the program is gonna
// abort (e.g. asserts)
// On Windows, this doesn't allocate at all, so it's safe to use whenever.
#define TYPENAME_TO_STRING(type) DemangledInfo{ typeid(type).name() }.leak_name()
