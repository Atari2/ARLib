#include "FileSystem.hpp"
namespace ARLib {
const Path& FileInfo::path() const {
    return m_info.path();
}
const FsStringView& FileInfo::filename() const {
    return m_info.filename();
}
bool FileInfo::is_directory() const {
    return m_info.is_directory();
}
bool FileInfo::is_file() const {
    return m_info.is_file();
}
size_t FileInfo::filesize() const {
    return m_info.filesize();
}
Nanos FileInfo::last_access() const {
    return m_info.last_access();
}
Nanos FileInfo::last_modification() const {
    return m_info.last_modification();
}
}    // namespace ARLib