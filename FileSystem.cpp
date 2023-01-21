#include "FileSystem.h"
namespace ARLib {
const Path& FileInfo::path() const {
#ifdef ON_WINDOWS
    return m_info.path();
#else
    return m_info.path();
#endif
}
}    // namespace ARLib