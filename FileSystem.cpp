#include "FileSystem.h"
namespace ARLib {
const Path& FileInfo::path() const {
#ifdef ON_WINDOWS
    return m_info.path();
#else
    return m_info.path();
#endif
}
const FsStringView& FileInfo::filename() const {
#ifdef ON_WINDOWS
    return m_info.filename();
#else
    return m_info.filename();
#endif
}
}    // namespace ARLib