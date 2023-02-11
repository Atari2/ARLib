#include "PrintInfo.h"
namespace ARLib {
String PrintInfo<SourceLocation>::repr() const {
    return String::formatted(
    "Function `%s` in file %s, at line %u:%u", m_loc.function_name(), m_loc.file_name(), m_loc.line(), m_loc.column()
    );
}
String PrintInfo<DemangledInfo>::repr() const {
    return String{ m_info.name() };
}
}    // namespace ARLib
