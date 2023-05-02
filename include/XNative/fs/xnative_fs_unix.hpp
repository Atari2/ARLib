#pragma once

#include "Compat.hpp"
#ifdef UNIX_OR_MINGW
    #if not defined(FILESYSTEM_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
        #error "Don't include the XNative files directly. Use FileSystem.h"
    #endif
    #include "Types.hpp"
    #include "Path.hpp"
    #include "UniquePtr.hpp"
    #include "Chrono.hpp"
struct stat;
namespace ARLib {
enum class UnixFileAttribute {};
class UnixFileInfo {
    friend class UnixDirectoryIterator;
    using StatHandle = struct ::stat*;
    mutable StatHandle statHandle{};
    StringView fileName{};
    Path fullPath{};
    void populate_stathandle() const;
    public:
    UnixFileInfo(const Path& path);
    UnixFileInfo(UnixFileInfo&& other) noexcept : fileName(), fullPath(move(other.fullPath)) {
        swap(statHandle, other.statHandle);
        auto idx_last_slash = fullPath.string().last_index_of('/');
        fileName            = fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
    }
    UnixFileInfo(const UnixFileInfo& other) noexcept : fileName(), fullPath(other.fullPath) {
        auto idx_last_slash = fullPath.string().last_index_of('/');
        fileName            = fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
    }
    UnixFileInfo& operator=(UnixFileInfo&& other) noexcept {
        fullPath            = move(other.fullPath);
        auto idx_last_slash = fullPath.string().last_index_of('/');
        fileName            = fullPath.string().substringview(idx_last_slash != WString::npos ? idx_last_slash + 1 : 0);
        return *this;
    }
    constexpr UnixFileInfo() = default;
    const auto& path() const { return fullPath; }
    const auto& filename() const { return fileName; }
    bool is_directory() const;
    bool is_file() const;
    size_t filesize() const;
    Nanos last_access() const;
    Nanos last_modification() const;
    ~UnixFileInfo();
};
using UnixDirIterHandle = void*;
bool remove_filespec(String& p);
bool is_directory(const String& p);
void parent_path(String& p);
bool is_absolute(const String& p);
FsString combine_paths(const FsString& p1, const FsString& p2);
class UnixDirectoryIterator;
class UnixDirectoryIterate {
    friend class UnixDirectoryIterator;
    Path m_path;
    UnixDirIterHandle m_glob_result;
    bool m_recurse;
    UnixDirectoryIterate() = default;
    UnixDirectoryIterate(UnixDirectoryIterate&& other) noexcept :
        m_path(move(other.m_path)), m_glob_result(other.m_glob_result), m_recurse(other.m_recurse) {
        other.m_glob_result = nullptr;
    }
    UnixDirectoryIterate& operator=(UnixDirectoryIterate&& other) noexcept;
    public:
    UnixDirectoryIterate(Path path, bool recurse);
    UnixDirectoryIterator begin() const;
    UnixDirectoryIterator end() const;
    ~UnixDirectoryIterate();
};
class UnixDirectoryIterator {
    friend class UnixDirectoryIterate;
    UnixDirIterHandle m_hdl;
    const Path& m_path;
    UnixFileInfo m_info;
    size_t m_index;
    bool m_recurse;
    UnixDirectoryIterate m_recursive_iterate{};
    UniquePtr<UnixDirectoryIterator> m_inner_curr;
    UniquePtr<UnixDirectoryIterator> m_inner_end;
    void load_next_file(bool first_time = false);
    UnixDirectoryIterator(const Path& path, bool recurse);
    UnixDirectoryIterator(const Path& path, UnixDirIterHandle hdl, bool recurse);
    public:
    UnixDirectoryIterator(const UnixDirectoryIterator&)            = delete;
    UnixDirectoryIterator& operator=(const UnixDirectoryIterator&) = delete;
    UnixDirectoryIterator(UnixDirectoryIterator&&) noexcept;
    UnixDirectoryIterator& operator=(UnixDirectoryIterator&&) noexcept = delete;
    bool operator==(const UnixDirectoryIterator& other) const;
    bool operator!=(const UnixDirectoryIterator& other) const;
    UnixFileInfo operator*() const;
    UnixDirectoryIterator& operator++();
    ~UnixDirectoryIterator();
};
}    // namespace ARLib
#endif
