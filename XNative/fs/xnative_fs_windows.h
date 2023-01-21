#pragma once
#include "../../Compat.h"
#ifdef WINDOWS
    #if not defined(FILESYSTEM_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
        #error "Don't include the XNative files directly. Use FileSystem.h"
    #endif
    #include "../../Types.h"
    #include "../../Path.h"
namespace ARLib {
enum class Win32FileAttribute {
    ARCHIVE               = 0x20,
    COMPRESSED            = 0x800,
    DEVICE                = 0x40,
    DIRECTORY             = 0x10,
    ENCRYPTED             = 0x4000,
    HIDDEN                = 0x2,
    INTEGRITY_STREAM      = 0x8000,
    NORMAL                = 0x80,
    NOT_CONTENT_INDEXED   = 0x2000,
    NO_SCRUB_DATA         = 0x20000,
    OFFLINE               = 0x1000,
    READONLY              = 0x1,
    RECALL_ON_DATA_ACCESS = 0x400000,
    RECALL_ON_OPEN        = 0x40000,
    REPARSE_POINT         = 0x400,
    SPARSE_FILE           = 0x200,
    SYSTEM                = 0x4,
    TEMPORARY             = 0x100,
    VIRTUAL               = 0x10000,
    PINNED                = 0x80000,
    UNPINNED              = 0x100000
};
class Win32FileInfo {
    friend class Win32DirectoryIterator;
    protected:
    uint32_t fileAttributes{};
    uint64_t creationTime{};
    uint64_t lastWrite{};
    uint64_t lastAccess{};
    uint64_t fileSize{};
    WStringView fileName{};
    Path fullPath{};
    public:
    constexpr Win32FileInfo() = default;
    const auto& path() const { return fullPath; }
    const auto& filename() const { return fileName; }
};
using Win32DirIterHandle = void*;
bool remove_filespec(WString& p);
class Win32DirectoryIterator {
    friend class Win32DirectoryIterate;
    Win32DirIterHandle m_hdl;
    const Path& m_path;
    Win32FileInfo& m_info;
    protected:
    Win32DirectoryIterator(const Path& path, Win32FileInfo& info);
    Win32DirectoryIterator(const Path& path, Win32DirIterHandle hdl, Win32FileInfo& info);
    public:
    Win32DirectoryIterator(const Win32DirectoryIterator&)  = delete;
    Win32DirectoryIterator& operator=(const Win32DirectoryIterator&) = delete;
    Win32DirectoryIterator(Win32DirectoryIterator&&) noexcept;
    Win32DirectoryIterator& operator=(Win32DirectoryIterator&&) noexcept = delete;
    bool operator==(const Win32DirectoryIterator& other) const;
    bool operator!=(const Win32DirectoryIterator& other) const;
    const Win32FileInfo& operator*() const;
    Win32DirectoryIterator& operator++();
    ~Win32DirectoryIterator();
};
class Win32DirectoryIterate {
    mutable Win32FileInfo m_info{};
    Path m_path;
    public:
    Win32DirectoryIterate(Path path) : m_path(move(path)){};
    auto begin() const { return Win32DirectoryIterator{ m_path, NULL, m_info }; }
    auto end() const { return Win32DirectoryIterator{ m_path, m_info }; }
};
}    // namespace ARLib
#endif
