#ifndef INCLUDED_FROM_OWN_CPP___
#define INCLUDED_FROM_OWN_CPP___
#endif
#include "XNative/fs/xnative_fs_windows.hpp"
#ifdef WINDOWS
#include "Printer.hpp"
#include <Windows.h>
#include <PathCch.h>
#pragma comment(lib, "Pathcch.lib")
namespace ARLib {
static auto merge_dwords(DWORD low, DWORD high) {
    return static_cast<uint64_t>(low) | (static_cast<uint64_t>(high) << (sizeof(DWORD) * CHAR_BIT));
};
Win32FileInfo::Win32FileInfo(const Path& path) {
    wchar_t* tmp_buf = nullptr;
    DWORD nChars     = GetFullPathName(path.string().data(), 0, tmp_buf, NULL);
    tmp_buf          = new wchar_t[nChars];
    GetFullPathName(path.string().data(), nChars, tmp_buf, NULL);
    fullPath            = WStringView{ tmp_buf };
    auto idx_last_slash = fullPath.string().last_index_of(L'\\');
    fileName            = fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
    WIN32_FIND_DATA data{};
    HANDLE hdl     = FindFirstFileW(tmp_buf, &data);
    fileAttributes = data.dwFileAttributes;
    lastAccess     = merge_dwords(data.ftLastAccessTime.dwLowDateTime, data.ftLastAccessTime.dwHighDateTime);
    lastWrite      = merge_dwords(data.ftLastWriteTime.dwLowDateTime, data.ftLastWriteTime.dwHighDateTime);
    creationTime   = merge_dwords(data.ftCreationTime.dwLowDateTime, data.ftCreationTime.dwHighDateTime);
    fileSize       = merge_dwords(data.nFileSizeLow, data.nFileSizeHigh);
    delete[] tmp_buf;
    FindClose(hdl);
}
Win32DirectoryIterate::Win32DirectoryIterate(Path path, bool recurse) : m_path(), m_recurse(recurse) {
    // make path a valid globbing path
    FsString str = path.string();
    if (path.is_directory()) {
        if (str.back() == L'\\') {
            str.append(L'*');
        } else {
            str.append(L"\\*");
        }
    }
    m_path = move(str);
}
Win32DirectoryIterator Win32DirectoryIterate::begin() const {
    return Win32DirectoryIterator{ m_path, NULL, m_recurse };
}
Win32DirectoryIterator Win32DirectoryIterate::end() const {
    return Win32DirectoryIterator{ m_path, m_recurse };
}
Win32DirectoryIterator::Win32DirectoryIterator(const Path& path, bool recurse) :
    m_hdl(INVALID_HANDLE_VALUE), m_path(&path), m_recurse(recurse) {
    // end-iterator constructor
}
Win32DirectoryIterator::Win32DirectoryIterator(const Path& path, Win32DirIterHandle hdl, bool recurse) :
    m_hdl(hdl), m_path(&path), m_recurse(recurse) {
    load_next_file();
}
Win32DirectoryIterator::Win32DirectoryIterator(Win32DirectoryIterator&& other) noexcept :
    m_hdl(other.m_hdl), m_path(other.m_path), m_info(move(other.m_info)), m_recurse(other.m_recurse),
    m_recursive_iterate(move(other.m_recursive_iterate)), m_inner_curr(move(other.m_inner_curr)),
    m_inner_end(move(other.m_inner_end)) {
    other.m_hdl = INVALID_HANDLE_VALUE;
    if (m_inner_curr.exists() && m_inner_end.exists()) {
        m_inner_curr->m_path = &m_recursive_iterate.m_path;
        m_inner_end->m_path  = &m_recursive_iterate.m_path;
    }
}
Win32DirectoryIterator::~Win32DirectoryIterator() {
    if (m_hdl != NULL && m_hdl != INVALID_HANDLE_VALUE) { (void)FindClose(m_hdl); }
}
Win32FileInfo Win32DirectoryIterator::operator*() const {
    if (m_inner_curr.exists()) { return m_inner_curr->operator*(); }
    return m_info;
}
static void construct_full_path(wchar_t* fullPathBuf, size_t bufSz, const Path& p) {
    GetCurrentDirectory(static_cast<DWORD>(bufSz), fullPathBuf);
    Path fp = p.remove_filespec();
    PathCchCombineEx(fullPathBuf, bufSz, fullPathBuf, fp.string().data(), PATHCCH_ALLOW_LONG_PATHS);
}
static Path combine_paths(const Path& p, FsStringView p2) {
    static wchar_t resultBuf[4096];
    Path p1 = p.remove_filespec();
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
        m_path->string().data(), FindExInfoBasic, &data, FindExSearchNameMatch, NULL,
        FIND_FIRST_EX_CASE_SENSITIVE | FIND_FIRST_EX_LARGE_FETCH
        );
        isDir = data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
    }
    if (m_hdl == INVALID_HANDLE_VALUE) { return; }
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
            m_recursive_iterate = Win32DirectoryIterate{ combine_paths(*m_path, nameView), m_recurse };
            m_inner_curr        = UniquePtr{ m_recursive_iterate.begin() };
            m_inner_end         = UniquePtr{ m_recursive_iterate.end() };
            return;
        }
    }
    m_info.fileAttributes = data.dwFileAttributes;
    m_info.creationTime   = merge_dwords(data.ftCreationTime.dwLowDateTime, data.ftCreationTime.dwHighDateTime);
    m_info.lastAccess     = merge_dwords(data.ftLastAccessTime.dwLowDateTime, data.ftLastAccessTime.dwHighDateTime);
    m_info.lastWrite      = merge_dwords(data.ftLastWriteTime.dwLowDateTime, data.ftLastWriteTime.dwHighDateTime);
    m_info.fileSize       = merge_dwords(data.nFileSizeLow, data.nFileSizeHigh);
    construct_full_path(fullPathBuf, bufSz, *m_path);
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
bool is_directory(const WString& p) {
    HANDLE hdl = CreateFile(p.data(), 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hdl == INVALID_HANDLE_VALUE) { return false; }
    FILE_BASIC_INFO info{};
    BOOL res = GetFileInformationByHandleEx(hdl, FileBasicInfo, &info, sizeof(FILE_BASIC_INFO));
    if (res) { return (info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; }
    return false;
}
// https://learn.microsoft.com/en-gb/windows/win32/fileio/naming-a-file#fully-qualified-vs-relative-paths
bool is_absolute(const WString& p) {
    // A file name is relative to the current directory (aka not absolute) if it does not begin with one of the following:
    if (p.starts_with(LR"(\\)")) {
        // - A UNC name of any format, which always start with two backslash characters ("\\"). For more information, see the next section.
        return true;
    } else if (p.substringview(1).starts_with(LR"(:\)")) {
        // - A disk designator with a backslash, for example "C:\" or "d:\".
        return true;
    } else if (p.starts_with(LR"(\)")) {
        // - A single backslash, for example, "\directory" or "\file.txt". This is also referred to as an absolute path.
        return true;
    }
    return false;
}
void parent_path(WString& p) {
    if (is_absolute(p)) {
        const size_t index_of_slash = p.last_index_of(L'\\');
        if (index_of_slash > 2 && index_of_slash != WString::npos) {
            // see: is_absolute
            p.resize(index_of_slash);
        }
    } else {
        const size_t index_of_slash = p.last_index_of(L'\\');
        p.resize(index_of_slash == WString::npos ? 0 : index_of_slash);
    }
}
FsString combine_paths(const FsString& p1, const FsString& p2) {
    FsString str{};
    str.reserve(p1.size() + p2.size());
    PathCchCombineEx(str.rawptr(), str.capacity(), p1.data(), p2.data(), PATHCCH_ALLOW_LONG_PATHS);
    str.set_size(wstrlen(str.data()));
    return str;
}
}    // namespace ARLib
#endif
