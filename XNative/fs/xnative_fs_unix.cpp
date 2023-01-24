#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "xnative_fs_unix.h"
#ifdef UNIX_OR_MINGW
    #include <cstdlib>
    #include <unistd.h>
    #include <sys/stat.h>
namespace ARLib {
UnixDirectoryIterate::UnixDirectoryIterate(Path path, bool recurse) : m_path(move(path)), m_glob_result{new GlobResult}, m_recurse(recurse) {
    struct stat pathStat;
    auto& str = m_path.string();
    int res = stat(str.data(), &pathStat);
    if (res < 0) {
        // TODO: find a way to handle failure here
        return;
    }
    if (S_ISDIR(pathStat.st_mode)) {
        if (str.back() == '/') {
            str.append('*');
        } else {
            str.append("/*");
        }
    }
};
UnixDirectoryIterator UnixDirectoryIterate::begin() const {
    return UnixDirectoryIterator{ m_path, m_glob_result, m_recurse };
}
UnixDirectoryIterator UnixDirectoryIterate::end() const {
    return UnixDirectoryIterator{ m_path, m_recurse };
}
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, bool recurse) :
    m_hdl(nullptr), m_path(path), m_index(0), m_recurse(recurse) {
    // end-iterator constructor
}
void UnixDirectoryIterator::load_next_file(bool first_time) {
    char pathBuf[PATH_MAX];
    pathBuf[0] = '\0'; 
    if (!first_time) {
        if (m_hdl == nullptr) return;
        if (m_hdl->gl_pathc == 0 || m_index >= m_hdl->gl_pathc) {
            // no paths or already done everything
            // set as end iterator
            globfree(m_hdl);
            m_hdl = NULL;
            m_index = 0;
            return;
        }
        if (m_inner_curr.exists() && m_inner_end.exists()) {
            auto& b = *m_inner_curr;
            auto& e = *m_inner_end;
            if (++b != e) {
                return;
            } else {
                m_inner_curr.reset();
                m_inner_end.reset();
            }
        }
        m_index++;
    } else {
        // first time we're called, we need to glob
        int res = glob(m_path.string().data(), GLOB_ERR, NULL, m_hdl);
        if (res != 0 || res == GLOB_NOMATCH || m_hdl->gl_pathc == 0) {
            // glob error or no matches
            // set as end-iterator and return
            globfree(m_hdl);
            m_hdl = NULL;
            m_index = 0;
            return;
        }
    }
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
            return;
        } else if (S_ISDIR(pathStat.st_mode) && m_recurse) {
            m_info.fullPath = Path{ StringView{ pathBuf } };
            m_info.fileName = m_info.fullPath.string().substringview(size + 1);
            auto nameView = StringView{ m_hdl->gl_pathv[m_index] };
            if (nameView == ".."_sv || nameView == "."_sv) {
                m_index++;    // skip . and .. directories
                continue;
            }
            m_recursive_iterate = UnixDirectoryIterate{ Path{ nameView }, m_recurse };
            auto b = m_recursive_iterate.begin();
            auto e = m_recursive_iterate.end();
            if (b != e) { 
                m_inner_curr        = UniquePtr{ move(b) };
                m_inner_end         = UniquePtr{ move(e) };
                return;
            }
        }
        m_index++;
    }
    globfree(m_hdl);
    m_hdl = NULL;
    m_index = 0;
}
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, UnixDirIterHandle hdl, bool recurse) :
    m_hdl(hdl), m_path(path), m_index(0), m_recurse(recurse) {
    load_next_file(true);
}
UnixDirectoryIterator::UnixDirectoryIterator(UnixDirectoryIterator&& other) noexcept :
    m_hdl(move(other.m_hdl)), m_path(move(other.m_path)), m_info(move(other.m_info)), m_index(other.m_index), m_recurse(other.m_recurse), 
    m_recursive_iterate(move(other.m_recursive_iterate)), m_inner_curr(move(other.m_inner_curr)), m_inner_end(move(other.m_inner_end)) {
    other.m_hdl = NULL;
}
UnixDirectoryIterator::~UnixDirectoryIterator() {
    if (m_hdl != nullptr) globfree(m_hdl);
    m_hdl = NULL;
}
UnixFileInfo UnixDirectoryIterator::operator*() const {
    if (m_inner_curr.exists()) { 
        return (*m_inner_curr).operator*(); 
    }
    return m_info;
}
UnixDirectoryIterator& UnixDirectoryIterator::operator++() {
    load_next_file();
    return *this;
}
bool UnixDirectoryIterator::operator==(const UnixDirectoryIterator& other) const {
    return other.m_hdl == this->m_hdl && other.m_index == this->m_index;
}
bool UnixDirectoryIterator::operator!=(const UnixDirectoryIterator& other) const {
    return other.m_hdl != this->m_hdl || other.m_index != this->m_index;
}
bool remove_filespec(String& p) {
    size_t last_slash = p.last_index_of('/');
    if (last_slash == String::npos) {
        p.set_size(0);
        return true;
    };
    p.set_size(last_slash);
    return true;
}
}    // namespace ARLib
#endif
