#define INCLUDED_FROM_OWN_CPP___
#include "xnative_fs_unix.h"
#ifdef UNIX_OR_MINGW
namespace ARLib {
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, UnixFileInfo& info) :
    m_hdl(nullptr), m_path(path), m_info(info) {
    // end-iterator constructor
}
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, UnixDirIterHandle hdl, UnixFileInfo& info) :
    m_hdl(hdl), m_path(path), m_info(info) {
    
}
UnixDirectoryIterator::UnixDirectoryIterator(UnixDirectoryIterator&& other) noexcept :
    m_hdl(other.m_hdl), m_path(other.m_path), m_info(other.m_info) {

}
UnixDirectoryIterator::~UnixDirectoryIterator() {
}
const UnixFileInfo& UnixDirectoryIterator::operator*() const {
    return m_info;
}
UnixDirectoryIterator& UnixDirectoryIterator::operator++() {
    return *this;
}
bool UnixDirectoryIterator::operator==(const UnixDirectoryIterator& other) const {
    return &this->m_path == &other.m_path && &this->m_info == &other.m_info && other.m_hdl == this->m_hdl;
}
bool UnixDirectoryIterator::operator!=(const UnixDirectoryIterator& other) const {
    return !(this->operator==(other));
}
bool remove_filespec(String& p) {
    return true;
}
}    // namespace ARLib
#endif
