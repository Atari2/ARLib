#pragma once
#include "../../Compat.h"
#ifdef WINDOWS
    #if not defined(FILESYSTEM_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
        #error "Don't include the XNative files directly. Use FileSystem.h"
    #endif
    #include "../../Types.h"
    #include "../../Path.h"
    #include "../../UniquePtr.h"
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
    Win32FileInfo(Win32FileInfo&& other) noexcept :
        fileAttributes(other.fileAttributes), creationTime(other.creationTime), lastWrite(other.lastWrite),
        lastAccess(other.lastAccess), fileSize(other.fileSize), fileName(), fullPath(move(other.fullPath)) {
        auto idx_last_slash = fullPath.string().last_index_of(L'\\');
        fileName            = fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
    }
    Win32FileInfo(const Win32FileInfo& other) noexcept :
        fileAttributes(other.fileAttributes), creationTime(other.creationTime), lastWrite(other.lastWrite),
        lastAccess(other.lastAccess), fileSize(other.fileSize), fileName(), fullPath(other.fullPath) {
        auto idx_last_slash = fullPath.string().last_index_of(L'\\');
        fileName            = fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
    }
    Win32FileInfo& operator=(Win32FileInfo&& other) noexcept {
        fileAttributes      = other.fileAttributes;
        creationTime        = other.creationTime;
        lastWrite           = other.lastWrite;
        lastAccess          = other.lastAccess;
        fileSize            = other.fileSize;
        fullPath            = move(other.fullPath);
        auto idx_last_slash = fullPath.string().last_index_of(L'\\');
        fileName            = fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
        return *this;
    }
    constexpr Win32FileInfo() = default;
    const auto& path() const { return fullPath; }
    const auto& filename() const { return fileName; }
    ~Win32FileInfo() = default;
};
using Win32DirIterHandle = void*;
bool remove_filespec(WString& p);
class Win32DirectoryIterate {
    friend class Win32DirectoryIterator;
    Path m_path;
    bool m_recurse;
    protected:
    Win32DirectoryIterate() : m_path(), m_recurse(){};
    Win32DirectoryIterate(Win32DirectoryIterate&& other) noexcept :
        m_path(move(other.m_path)), m_recurse(other.m_recurse) {}
    Win32DirectoryIterate& operator=(Win32DirectoryIterate&& other) noexcept {
        m_path    = move(other.m_path);
        m_recurse = other.m_recurse;
        return *this;
    }
    void set_values_from_iterator(Path path, bool recurse);
    public:
    Win32DirectoryIterate(Path path, bool recurse = false);
    Win32DirectoryIterator begin() const;
    Win32DirectoryIterator end() const;
};
class Win32DirectoryIterator {
    friend Win32DirectoryIterate;
    friend struct PrintInfo<Win32DirectoryIterator>;
    Win32DirIterHandle m_hdl;
    const Path& m_path;
    Win32FileInfo m_info;
    const bool m_recurse;
    Win32DirectoryIterate m_recursive_iterate{};
    UniquePtr<Win32DirectoryIterator> m_inner_curr;
    UniquePtr<Win32DirectoryIterator> m_inner_end;
    void load_next_file();
    protected:
    Win32DirectoryIterator(const Path& path, bool recurse);
    Win32DirectoryIterator(const Path& path, Win32DirIterHandle hdl, bool recurse);
    public:
    Win32DirectoryIterator(const Win32DirectoryIterator&)            = delete;
    Win32DirectoryIterator& operator=(const Win32DirectoryIterator&) = delete;
    Win32DirectoryIterator(Win32DirectoryIterator&&) noexcept;
    Win32DirectoryIterator& operator=(Win32DirectoryIterator&&) noexcept = delete;
    bool operator==(const Win32DirectoryIterator& other) const;
    bool operator!=(const Win32DirectoryIterator& other) const;
    Win32FileInfo operator*() const;
    Win32DirectoryIterator& operator++();
    ~Win32DirectoryIterator();
};
template <>
struct PrintInfo<Win32DirectoryIterator> {
    const Win32DirectoryIterator& m_iter;
    String repr() const {
        auto end = Win32DirectoryIterator {m_iter.m_path, m_iter.m_recurse};
        if (m_iter == end) { return "Win32DirectoryIterator { end }"_s; }
        return (*m_iter).path().narrow();
    }
};
}    // namespace ARLib
#endif
