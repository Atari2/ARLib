#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "XNative/fs/xnative_fs_unix.hpp"
#ifdef UNIX_OR_MINGW
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <glob.h>
#include <stdio.h>
#include "Chrono.hpp"
namespace ARLib {
UnixFileInfo::UnixFileInfo(const Path& path) {
    char buffer[PATH_MAX] = { 0 };
    realpath(path.string().data(), buffer);
    fullPath            = StringView{ buffer };
    auto idx_last_slash = fullPath.string().last_index_of('/');
    fileName            = fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
    populate_stathandle();
}
void UnixFileInfo::populate_stathandle() const {
    if (statHandle == nullptr) {
        statHandle = new struct stat;
        int ret    = stat(fullPath.string().data(), statHandle);
        if (ret != 0) { ::perror("stat"); }
    }
}
bool UnixFileInfo::is_directory() const {
    return S_ISDIR(statHandle->st_mode);
}
bool UnixFileInfo::is_file() const {
    return S_ISREG(statHandle->st_mode);
}
size_t UnixFileInfo::filesize() const {
    return static_cast<size_t>(statHandle->st_size);
}
Nanos UnixFileInfo::last_access() const {
    return Seconds{ statHandle->st_atim.tv_sec } + Nanos{ statHandle->st_atim.tv_nsec };
}
Nanos UnixFileInfo::last_modification() const {
    return Seconds{ statHandle->st_atim.tv_sec } + Nanos{ statHandle->st_atim.tv_nsec };
}
UnixFileInfo::~UnixFileInfo() {
    if (statHandle) delete statHandle;
}
UnixDirectoryIterate::UnixDirectoryIterate(Path path, bool recurse) :
    m_path(), m_glob_result{ new glob_t }, m_recurse(recurse) {
    FsString str = path.string();
    if (path.is_directory()) {
        if (str.back() == '/') {
            str.append('*');
        } else {
            str.append("/*");
        }
    }
    m_path = move(str);
};
UnixDirectoryIterator UnixDirectoryIterate::begin() const {
    return UnixDirectoryIterator{ m_path, m_glob_result, m_recurse };
}
UnixDirectoryIterator UnixDirectoryIterate::end() const {
    return UnixDirectoryIterator{ m_path, m_recurse };
}
UnixDirectoryIterate& UnixDirectoryIterate::operator=(UnixDirectoryIterate&& other) noexcept {
    if (m_glob_result) delete static_cast<glob_t*>(m_glob_result);
    m_path              = move(other.m_path);
    m_recurse           = other.m_recurse;
    m_glob_result       = other.m_glob_result;
    other.m_glob_result = nullptr;
    return *this;
}
UnixDirectoryIterate::~UnixDirectoryIterate() {
    if (m_glob_result) delete static_cast<glob_t*>(m_glob_result);
}
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, bool recurse) :
    m_hdl(nullptr), m_path(path), m_index(0), m_recurse(recurse) {
    // end-iterator constructor
}
void UnixDirectoryIterator::load_next_file(bool first_time) {
    char pathBuf[PATH_MAX];
    pathBuf[0]  = '\0';
    glob_t* hdl = static_cast<glob_t*>(this->m_hdl);
    if (!first_time) {
        if (hdl == nullptr) return;
        if (hdl->gl_pathc == 0 || m_index >= hdl->gl_pathc) {
            // no paths or already done everything
            // set as end iterator
            globfree(hdl);
            hdl     = NULL;
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
        int res = glob(m_path.string().data(), GLOB_ERR, NULL, hdl);
        if (res != 0 || res == GLOB_NOMATCH || hdl->gl_pathc == 0) {
            // glob error or no matches
            // set as end-iterator and return
            globfree(hdl);
            hdl     = NULL;
            m_index = 0;
            return;
        }
    }
    size_t size = 0;
    if (!m_path.is_absolute()) {
        getcwd(pathBuf, PATH_MAX);
        size            = strlen(pathBuf);
        pathBuf[size]   = '/';
        pathBuf[++size] = '\0';
    }
    while (m_index < hdl->gl_pathc) {
        strcat(pathBuf, hdl->gl_pathv[m_index]);
        struct stat pathStat;
        stat(pathBuf, &pathStat);
        if (S_ISREG(pathStat.st_mode)) {
            m_info.fullPath = Path{ StringView{ pathBuf } };
            m_info.fileName = m_info.fullPath.string().substringview(size + 1);
            return;
        } else if (S_ISDIR(pathStat.st_mode) && m_recurse) {
            m_info.fullPath = Path{ StringView{ pathBuf } };
            m_info.fileName = m_info.fullPath.string().substringview(size + 1);
            auto nameView   = StringView{ hdl->gl_pathv[m_index] };
            if (nameView == ".."_sv || nameView == "."_sv) {
                m_index++;    // skip . and .. directories
                continue;
            }
            m_recursive_iterate = UnixDirectoryIterate{ Path{ nameView }, m_recurse };
            auto b              = m_recursive_iterate.begin();
            auto e              = m_recursive_iterate.end();
            if (b != e) {
                m_inner_curr = UniquePtr{ move(b) };
                m_inner_end  = UniquePtr{ move(e) };
                return;
            }
        }
        pathBuf[size] = '\0';
        m_index++;
    }
    globfree(hdl);
    hdl     = NULL;
    m_index = 0;
}
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, UnixDirIterHandle hdl, bool recurse) :
    m_hdl(hdl), m_path(path), m_index(0), m_recurse(recurse) {
    load_next_file(true);
}
UnixDirectoryIterator::UnixDirectoryIterator(UnixDirectoryIterator&& other) noexcept :
    m_hdl(move(other.m_hdl)), m_path(move(other.m_path)), m_info(move(other.m_info)), m_index(other.m_index),
    m_recurse(other.m_recurse), m_recursive_iterate(move(other.m_recursive_iterate)),
    m_inner_curr(move(other.m_inner_curr)), m_inner_end(move(other.m_inner_end)) {
    other.m_hdl = NULL;
}
UnixDirectoryIterator::~UnixDirectoryIterator() {
    if (m_hdl != nullptr) globfree(static_cast<glob_t*>(m_hdl));
    m_hdl = NULL;
}
UnixFileInfo UnixDirectoryIterator::operator*() const {
    if (m_inner_curr.exists()) { return (*m_inner_curr).operator*(); }
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
bool is_directory(const String& p) {
    struct stat st {};
    stat(p.data(), &st);
    return S_ISDIR(st.st_mode);
}
bool is_absolute(const String& p) {
    if (!p.is_empty())
        return p[0] == '/';
    else
        return false;
}
void parent_path(String& p) {
    const size_t slash_index = p.last_index_of('/');
    if (slash_index > 0) {
        // last slash is not at pos 0 (we're at root) and it exists somewhere
        p.resize(slash_index == String::npos ? 0 : slash_index);
    }
}
FsString combine_paths(const FsString& p1, const FsString& p2) {
    if (is_absolute(p2)) { return p2; }
    if (p1.back() != '/') {
        // has filename
        return p1 + '/' + p2;
    } else {
        return p1 + p2;
    }
}
}    // namespace ARLib
#endif
