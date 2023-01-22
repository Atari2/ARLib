#define INCLUDED_FROM_OWN_CPP___
#include "xnative_fs_unix.h"
#ifdef UNIX_OR_MINGW
    #include <cstdlib>
    #include <unistd.h>
    #include <sys/stat.h>
namespace ARLib {
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, UnixFileInfo& info) :
    m_hdl(nullptr), m_path(path), m_info(info), m_index(0) {
    // end-iterator constructor
}
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, UnixDirIterHandle hdl, UnixFileInfo& info) :
    m_hdl(hdl), m_path(path), m_info(info), m_index(0) {
    int res = glob(m_path.string().data(), GLOB_ERR, NULL, m_hdl);
    if (res != 0 || res == GLOB_NOMATCH || m_hdl->gl_pathc == 0) {
        globfree(m_hdl);
        m_hdl = nullptr;
        return;
    }   
    static char pathBuf[PATH_MAX];
    getcwd(pathBuf, PATH_MAX);
    size_t size       = strlen(pathBuf);
    pathBuf[size]     = '/';
    pathBuf[size + 1] = '\0';
    while (m_index < m_hdl->gl_pathc) { 
        strcat(pathBuf, m_hdl->gl_pathv[m_index]);        
        struct stat pathStat;
        stat(pathBuf, &pathStat);
        if (S_ISREG(pathStat.st_mode)) {
            m_info.fullPath = Path{ StringView{ pathBuf } };
            m_info.fileName = m_info.fullPath.string().substringview(size + 1);
            break;
        }
        m_index++;
    }
    if (m_index >= m_hdl->gl_pathc) {
        globfree(m_hdl);
        m_hdl   = nullptr;
        m_index = 0;
    }
}
UnixDirectoryIterator::UnixDirectoryIterator(UnixDirectoryIterator&& other) noexcept :
    m_hdl(other.m_hdl), m_path(other.m_path), m_info(other.m_info) {
    other.m_hdl = nullptr;
}
UnixDirectoryIterator::~UnixDirectoryIterator() {
    if (m_hdl != nullptr) globfree(m_hdl);
    m_hdl = nullptr;
}
const UnixFileInfo& UnixDirectoryIterator::operator*() const {
    return m_info;
}
UnixDirectoryIterator& UnixDirectoryIterator::operator++() {
    if (m_hdl == nullptr) return *this;
    m_index++;
    if (m_hdl->gl_pathc == 0 || m_index >= m_hdl->gl_pathc) {
        globfree(m_hdl);
        m_hdl   = nullptr;
        m_index = 0;
        return *this;
    } else {
        static char pathBuf[PATH_MAX];
        getcwd(pathBuf, PATH_MAX);
        size_t size       = strlen(pathBuf);
        pathBuf[size]     = '/';
        pathBuf[size + 1] = '\0';
        while (m_index < m_hdl->gl_pathc) { 
            strcat(pathBuf, m_hdl->gl_pathv[m_index]);        
            struct stat pathStat;
            stat(pathBuf, &pathStat);
            if (S_ISREG(pathStat.st_mode)) {
                m_info.fullPath = Path{ StringView{ pathBuf } };
                m_info.fileName = m_info.fullPath.string().substringview(size + 1);
                break;
            }
            m_index++;
        }
        if (m_index >= m_hdl->gl_pathc) {
            globfree(m_hdl);
            m_hdl   = nullptr;
            m_index = 0;
        }
    }
    return *this;
}
bool UnixDirectoryIterator::operator==(const UnixDirectoryIterator& other) const {
    return &this->m_path == &other.m_path && &this->m_info == &other.m_info && other.m_hdl == this->m_hdl &&
           other.m_index == this->m_index;
}
bool UnixDirectoryIterator::operator!=(const UnixDirectoryIterator& other) const {
    return !(this->operator==(other));
}
bool remove_filespec(String& p) {
    // TODO: implement
    return true;
}
}    // namespace ARLib
#endif
