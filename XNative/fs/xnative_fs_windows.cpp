#define INCLUDED_FROM_OWN_CPP___
#include "xnative_fs_windows.h"
#ifdef WINDOWS
    #include <Windows.h>
    #include <PathCch.h>
    #pragma comment(lib, "Pathcch.lib")
namespace ARLib {
Win32DirectoryIterator::Win32DirectoryIterator(const Path& path, Win32FileInfo& info) :
    m_hdl(INVALID_HANDLE_VALUE), m_path(path), m_info(info) {
    // end-iterator constructor
}
static auto merge_dwords(DWORD low, DWORD high) {
    return static_cast<uint64_t>(low) | (static_cast<uint64_t>(high) << (sizeof(DWORD) * CHAR_BIT));
};
Win32DirectoryIterator::Win32DirectoryIterator(const Path& path, Win32DirIterHandle hdl, Win32FileInfo& info) :
    m_hdl(hdl), m_path(path), m_info(info) {
    static wchar_t fullPathBuf[4096];
    constexpr size_t bufSz = sizeof_array(fullPathBuf);
    WIN32_FIND_DATA data{};
    m_hdl = FindFirstFileEx(
    m_path.string().data(), FindExInfoBasic, &data, FindExSearchNameMatch, NULL,
    FIND_FIRST_EX_CASE_SENSITIVE | FIND_FIRST_EX_LARGE_FETCH
    );
    if (m_hdl == INVALID_HANDLE_VALUE) { return; }
    m_info.fileAttributes = data.dwFileAttributes;
    m_info.creationTime   = merge_dwords(data.ftCreationTime.dwLowDateTime, data.ftCreationTime.dwHighDateTime);
    m_info.lastAccess     = merge_dwords(data.ftLastAccessTime.dwLowDateTime, data.ftLastAccessTime.dwHighDateTime);
    m_info.lastWrite      = merge_dwords(data.ftLastWriteTime.dwLowDateTime, data.ftLastWriteTime.dwHighDateTime);
    m_info.fileSize       = merge_dwords(data.nFileSizeLow, data.nFileSizeHigh);
    GetCurrentDirectory(bufSz, fullPathBuf);
    Path p{ m_path };
    p.remove_filespec();
    PathCchCombineEx(fullPathBuf, bufSz, fullPathBuf, p.string().data(), PATHCCH_ALLOW_LONG_PATHS);
    auto res = PathCchCombineEx(fullPathBuf, bufSz, fullPathBuf, data.cFileName, PATHCCH_ALLOW_LONG_PATHS);
    if (res != S_OK) {
        m_info.fullPath = WString{ data.cFileName };
        m_info.fileName = m_info.fullPath.string().view();
    } else {
        m_info.fullPath     = WString{ fullPathBuf };
        auto idx_last_slash = m_info.fullPath.string().last_index_of(L'\\');
        m_info.fileName = m_info.fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash : 0);
    }
}
Win32DirectoryIterator::Win32DirectoryIterator(Win32DirectoryIterator&& other) noexcept :
    m_hdl(other.m_hdl), m_path(other.m_path), m_info(other.m_info) {
    other.m_hdl = INVALID_HANDLE_VALUE;
}
Win32DirectoryIterator::~Win32DirectoryIterator() {
    if (m_hdl != NULL && m_hdl != INVALID_HANDLE_VALUE) { (void)FindClose(m_hdl); }
}
const Win32FileInfo& Win32DirectoryIterator::operator*() const {
    return m_info;
}
Win32DirectoryIterator& Win32DirectoryIterator::operator++() {
    static wchar_t fullPathBuf[4096];
    constexpr size_t bufSz = sizeof_array(fullPathBuf);
    if (m_hdl == INVALID_HANDLE_VALUE) { return *this; }
    WIN32_FIND_DATA data{};
    if (BOOL res = FindNextFile(m_hdl, &data); res == FALSE) {
        if (GetLastError() == ERROR_NO_MORE_FILES) { m_hdl = INVALID_HANDLE_VALUE; }
        FindClose(m_hdl);
        return *this;
    }
    m_info.fileAttributes = data.dwFileAttributes;
    m_info.creationTime   = merge_dwords(data.ftCreationTime.dwLowDateTime, data.ftCreationTime.dwHighDateTime);
    m_info.lastAccess     = merge_dwords(data.ftLastAccessTime.dwLowDateTime, data.ftLastAccessTime.dwHighDateTime);
    m_info.lastWrite      = merge_dwords(data.ftLastWriteTime.dwLowDateTime, data.ftLastWriteTime.dwHighDateTime);
    m_info.fileSize       = merge_dwords(data.nFileSizeLow, data.nFileSizeHigh);
    GetCurrentDirectory(bufSz, fullPathBuf);
    Path p{ m_path };
    p.remove_filespec();
    PathCchCombineEx(fullPathBuf, bufSz, fullPathBuf, p.string().data(), PATHCCH_ALLOW_LONG_PATHS);
    auto res = PathCchCombineEx(fullPathBuf, bufSz, fullPathBuf, data.cFileName, PATHCCH_ALLOW_LONG_PATHS);
    if (res != S_OK) {
        m_info.fullPath = WString{ data.cFileName };
        m_info.fileName = m_info.fullPath.string().view();
    } else {
        m_info.fullPath     = WString{ fullPathBuf };
        auto idx_last_slash = m_info.fullPath.string().last_index_of(L'\\');
        m_info.fileName = m_info.fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash : 0);
    }
    return *this;
}
bool Win32DirectoryIterator::operator==(const Win32DirectoryIterator& other) const {
    return &this->m_path == &other.m_path && &this->m_info == &other.m_info && other.m_hdl == this->m_hdl;
}
bool Win32DirectoryIterator::operator!=(const Win32DirectoryIterator& other) const {
    return !(this->operator==(other));
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
