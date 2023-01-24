#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "xnative_fs_windows.h"
#ifdef WINDOWS
    #include "../../Printer.h"
    #include <Windows.h>
    #include <PathCch.h>
    #include <Shlwapi.h>
    #pragma comment(lib, "Pathcch.lib")
    #pragma comment(lib, "Shlwapi.lib")
namespace ARLib {
Win32DirectoryIterate::Win32DirectoryIterate(Path path, bool recurse) : m_path(move(path)), m_recurse(recurse) {
    // make path a valid globbing path
    auto& str        = m_path.string();
    const bool isDir = PathIsDirectory(str.data());
    if (isDir) {
        if (str.back() == '\\') {
            str.append(L'*');
        } else {
            str.append(L"\\*");
        }
    }
}
Win32DirectoryIterator Win32DirectoryIterate::begin() const {
    return Win32DirectoryIterator{ m_path, NULL, m_recurse };
}
Win32DirectoryIterator Win32DirectoryIterate::end() const {
    return Win32DirectoryIterator{ m_path, m_recurse };
}
Win32DirectoryIterator::Win32DirectoryIterator(const Path& path, bool recurse) :
    m_hdl(INVALID_HANDLE_VALUE), m_path(path), m_recurse(recurse) {
    // end-iterator constructor
}
static auto merge_dwords(DWORD low, DWORD high) {
    return static_cast<uint64_t>(low) | (static_cast<uint64_t>(high) << (sizeof(DWORD) * CHAR_BIT));
};
Win32DirectoryIterator::Win32DirectoryIterator(const Path& path, Win32DirIterHandle hdl, bool recurse) :
    m_hdl(hdl), m_path(path), m_recurse(recurse) {
    load_next_file();
}
Win32DirectoryIterator::Win32DirectoryIterator(Win32DirectoryIterator&& other) noexcept :
    m_hdl(other.m_hdl), m_path(other.m_path), m_info(move(other.m_info)), m_recurse(other.m_recurse),
    m_recursive_iterate(move(other.m_recursive_iterate)), m_inner_curr(move(other.m_inner_curr)),
    m_inner_end(move(other.m_inner_end)) {
    other.m_hdl = INVALID_HANDLE_VALUE;
}
Win32DirectoryIterator::~Win32DirectoryIterator() {
    if (m_hdl != NULL && m_hdl != INVALID_HANDLE_VALUE) { (void)FindClose(m_hdl); }
}
Win32FileInfo Win32DirectoryIterator::operator*() const {
    if (m_inner_curr.exists()) { return m_inner_curr->operator*(); }
    return m_info;
}
static void construct_full_path(wchar_t* fullPathBuf, size_t bufSz, const Path& p) {
    GetCurrentDirectory(bufSz, fullPathBuf);
    Path fp{ p };
    fp.remove_filespec();
    PathCchCombineEx(fullPathBuf, bufSz, fullPathBuf, fp.string().data(), PATHCCH_ALLOW_LONG_PATHS);
}
static Path combine_paths(Path p1, FsStringView p2) {
    static wchar_t resultBuf[4096];
    p1.remove_filespec();
    PathCchCombineEx(resultBuf, sizeof_array(resultBuf), p1.string().data(), p2.data(), PATHCCH_ALLOW_LONG_PATHS);
    return Path{ WString{ resultBuf } };
}
void Win32DirectoryIterator::load_next_file() {
    static wchar_t fullPathBuf[4096];
    constexpr size_t bufSz = sizeof_array(fullPathBuf);
    WIN32_FIND_DATA data{};
    bool isDir = true;
    if (m_hdl == NULL) {
        // first time load_next_file is called
        m_hdl = FindFirstFileEx(
        m_path.string().data(), FindExInfoBasic, &data, FindExSearchNameMatch, NULL,
        FIND_FIRST_EX_CASE_SENSITIVE | FIND_FIRST_EX_LARGE_FETCH
        );
        isDir = data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    }
    if (m_hdl == INVALID_HANDLE_VALUE) { return; }
    while (isDir) {
        if (BOOL res = FindNextFile(m_hdl, &data); res == FALSE) {
            if (GetLastError() == ERROR_NO_MORE_FILES) { m_hdl = INVALID_HANDLE_VALUE; }
            FindClose(m_hdl);
            return;
        } else if (data.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY) {
            // we found a file
            isDir = false;
        } else if (m_recurse) {
            auto nameView = WStringView{ data.cFileName };
            if (nameView == L".."_wsv || nameView == L"."_wsv) {
                // skip dot-dot and dot
                continue;
            }
            // we found a directory and we're recursing
            m_recursive_iterate = Win32DirectoryIterate{ combine_paths(m_path, nameView), m_recurse };
            m_inner_curr        = UniquePtr{ m_recursive_iterate.begin() };
            m_inner_end         = UniquePtr{ m_recursive_iterate.end() };
            return;
        }
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
    m_info.fileAttributes = data.dwFileAttributes;
    m_info.creationTime   = merge_dwords(data.ftCreationTime.dwLowDateTime, data.ftCreationTime.dwHighDateTime);
    m_info.lastAccess     = merge_dwords(data.ftLastAccessTime.dwLowDateTime, data.ftLastAccessTime.dwHighDateTime);
    m_info.lastWrite      = merge_dwords(data.ftLastWriteTime.dwLowDateTime, data.ftLastWriteTime.dwHighDateTime);
    m_info.fileSize       = merge_dwords(data.nFileSizeLow, data.nFileSizeHigh);
    construct_full_path(fullPathBuf, bufSz, m_path);
    auto res = PathCchCombineEx(fullPathBuf, bufSz, fullPathBuf, data.cFileName, PATHCCH_ALLOW_LONG_PATHS);
    if (res != S_OK) {
        m_info.fullPath = WString{ data.cFileName };
        m_info.fileName = m_info.fullPath.string().view();
    } else {
        m_info.fullPath     = WString{ fullPathBuf };
        auto idx_last_slash = m_info.fullPath.string().last_index_of(L'\\');
        m_info.fileName =
        m_info.fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
    }
}
Win32DirectoryIterator& Win32DirectoryIterator::operator++() {
    load_next_file();
    return *this;
}
bool Win32DirectoryIterator::operator==(const Win32DirectoryIterator& other) const {
    return other.m_hdl == m_hdl;
}
bool Win32DirectoryIterator::operator!=(const Win32DirectoryIterator& other) const {
    return other.m_hdl != m_hdl;
}
bool remove_filespec(WString& p) {
    auto res = PathCchRemoveFileSpec(p.rawptr(), p.size());
    if (res != S_OK) { return false; }
    auto newsize = wstrlen(p.data());
    p.set_size(newsize);
    return true;
}
}    // namespace ARLib
#endif
