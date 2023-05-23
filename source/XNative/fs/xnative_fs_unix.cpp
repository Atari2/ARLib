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
Instant UnixFileInfo::last_access() const {
    return Instant::from_nanos(Seconds{ statHandle->st_atim.tv_sec } + Nanos{ statHandle->st_atim.tv_nsec });
}
Instant UnixFileInfo::last_modification() const {
    return Instant::from_nanos(Seconds{ statHandle->st_atim.tv_sec } + Nanos{ statHandle->st_atim.tv_nsec });
}
UnixFileInfo::~UnixFileInfo() {
    if (statHandle) delete statHandle;
}
void UnixFileInfo::copy_from_stathandle(const struct ::stat& handle) {
    free_stathandle();
    statHandle = new struct ::stat;
    memcpy(statHandle, &handle, sizeof(struct ::stat));
}
void UnixFileInfo::free_stathandle() {
    if (statHandle) delete statHandle;
    statHandle = nullptr;
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
    m_hdl(nullptr), m_path(&path), m_index(0), m_recurse(recurse) {
    // end-iterator constructor
}

void UnixDirectoryIterator::set_entry_info() {
    glob_t* hdl = static_cast<glob_t*>(m_hdl);
    static char pathBuf[PATH_MAX];
    pathBuf[0]  = '\0';
    if (!m_path->is_absolute()) {
        getcwd(pathBuf, PATH_MAX);
        size_t size            = strlen(pathBuf);
        pathBuf[size]   = '/';
        pathBuf[++size] = '\0';
    }
    strcat(pathBuf, hdl->gl_pathv[m_index]);
    struct stat pathStat;
    stat(pathBuf, &pathStat);
    m_info.fullPath = Path{ StringView{ pathBuf } };
    size_t last_slash = m_info.fullPath.string().last_index_of('/');
    m_info.fileName = m_info.fullPath.string().substringview(last_slash != String::npos ? last_slash + 1 : 0);
    m_info.copy_from_stathandle(pathStat);
}
bool UnixDirectoryIterator::find_next_not_dot_or_dot_dot() {
    glob_t* hdl = static_cast<glob_t*>(m_hdl);
    while (m_index < hdl->gl_pathc) {
        const char* pv = hdl->gl_pathv[m_index];
        auto nameView = StringView{pv};
        if (nameView != ".."_sv && nameView != "."_sv) { return true; };
        m_index++;
    }
    return false;
}
void UnixDirectoryIterator::load_first_file() {
    glob_t* hdl = static_cast<glob_t*>(this->m_hdl);
    int res = glob(m_path->string().data(), GLOB_ERR, NULL, hdl);
    if (res != 0 || hdl->gl_pathc == 0) {
        // glob error or no matches
        // set as end-iterator and return
        globfree(hdl);
        m_hdl     = NULL;
        m_index = 0;
        m_state = State::Finished;
    } else {
        if (!find_next_not_dot_or_dot_dot()) {
            globfree(hdl);
            m_hdl = NULL;
            m_index = 0;
            m_state = State::Finished;
        } else {
            set_entry_info();
            bool isDir = S_ISDIR(m_info.statHandle->st_mode);
            m_state = isDir ? State::Directory : State::File;
        }
    }

}

static Path combine_paths(const Path& p, FsStringView p2) {
    return p.remove_filespec() / Path{p2};
}

void UnixDirectoryIterator::load_next_file() {
    glob_t* hdl = static_cast<glob_t*>(this->m_hdl);

    HARD_ASSERT(m_state != State::Uninitialized, "State should be initialized when load_next_file is called");

    if (m_state == State::Finished) {
        return;
    }

    if (m_state == State::Recursing) {
        HARD_ASSERT(m_inner_curr.exists(), "Inner should exist when recursing");
        HARD_ASSERT(m_inner_end.exists(), "Inner end should exist when recursing");
        auto& b = *m_inner_curr;
        auto& e = *m_inner_end;
        if (++b != e) {
            return;
        } else {
            m_inner_curr.reset();
            m_inner_end.reset();
            m_state = State::File;
        }
    }

    if (hdl->gl_pathc == 0 || m_index >= hdl->gl_pathc) {
        // no paths or already done everything
        // set as end iterator
        globfree(hdl);
        m_hdl     = NULL;
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
    if (m_state == State::File || (m_state == State::Directory && !m_recurse)) {
        // normal file or directory and we're not asked to recurse, load + return next
        m_index++;
        if (!find_next_not_dot_or_dot_dot()) {
            globfree(hdl);
            m_hdl   = NULL;
            m_state = State::Finished;
            m_index = 0;
        } else {
            set_entry_info();
            bool isDir = S_ISDIR(m_info.statHandle->st_mode);
            m_state = isDir ? State::Directory : State::File;
        }
    } else if (m_state == State::Directory && m_recurse) {
        // we found a directory and we're recursing
        m_recursive_iterate = UnixDirectoryIterate{ combine_paths(*m_path, m_info.fileName), m_recurse };
        m_inner_curr        = UniquePtr{ m_recursive_iterate.begin() };
        m_inner_end         = UniquePtr{ m_recursive_iterate.end() };
        m_state             = State::Recursing;
    } else {
        HARD_ASSERT(false, "Unreachable");
    }
}
UnixDirectoryIterator::UnixDirectoryIterator(const Path& path, UnixDirIterHandle hdl, bool recurse) :
    m_hdl(hdl), m_path(&path), m_index(0), m_recurse(recurse), m_state(State::Uninitialized) {
    load_first_file();
}
UnixDirectoryIterator::UnixDirectoryIterator(UnixDirectoryIterator&& other) noexcept :
    m_hdl(move(other.m_hdl)), m_path(move(other.m_path)), m_info(move(other.m_info)), m_index(other.m_index),
    m_recurse(other.m_recurse), m_state(other.m_state),
    m_recursive_iterate(move(other.m_recursive_iterate)), m_inner_curr(move(other.m_inner_curr)), m_inner_end(move(other.m_inner_end)) {
    other.m_hdl = NULL;
    if (m_inner_curr.exists() && m_inner_end.exists()) {
        m_inner_curr->m_path = &m_recursive_iterate.m_path;
        m_inner_end->m_path  = &m_recursive_iterate.m_path;
    }
}
UnixDirectoryIterator::~UnixDirectoryIterator() {
    if (m_hdl != nullptr) globfree(static_cast<glob_t*>(m_hdl));
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
