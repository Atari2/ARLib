#include "TypeInfo.h"

#ifdef ON_LINUX
    #include <cstdlib>
    #include <cxxabi.h>
#endif
namespace ARLib {
DemangledInfo::DemangledInfo(const TypeInfo& info, bool critical) {
    const char* mangled_name = info.name();
#ifdef ON_LINUX
    int status       = 0;
    m_demangled_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
    if (status != 0) {
        // if there's any errors, we just use the mangled name (and we set it to not be freed)
        m_demangled_name = mangled_name;
        m_should_dealloc = false;
    } else {
        // if no errors, check if we're in a critical situation (e.g. we're aborting because an assert failed)
        // if so, we don't care about freeing the buffer, so we don't do it.
        m_should_dealloc = !critical;
    }
#else
    // on msvc names are already demangled.
    m_should_dealloc = false;
    m_demangled_name = mangled_name;
#endif
}
DemangledInfo::DemangledInfo(const char* mangled_name, bool critical) {
#ifdef ON_LINUX
    int status       = 0;
    m_demangled_name = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
    if (status != 0) {
        // if there's any errors, we just use the mangled name (and we set it to not be freed)
        m_demangled_name = mangled_name;
        m_should_dealloc = false;
    } else {
        // if no errors, check if we're in a critical situation (e.g. we're aborting because an assert failed)
        // if so, we don't care about freeing the buffer, so we don't do it.
        m_should_dealloc = !critical;
    }
#else
    // on msvc names are already demangled.
    m_should_dealloc = false;
    m_demangled_name = mangled_name;
#endif
}
DemangledInfo::~DemangledInfo() {
#ifdef ON_LINUX
    if (m_should_dealloc) std::free(const_cast<char*>(m_demangled_name));
#endif
}
}    // namespace ARLib
