#pragma once

#include "../../Compat.h"
#ifdef UNIX_OR_MINGW
    #if not defined(FILESYSTEM_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
        #error "Don't include the XNative files directly. Use FileSystem.h"
    #endif
    #include "../../Types.h"
    #include "../../Path.h"
    #include <dirent.h>
    #include <glob.h>
namespace ARLib {
enum class UnixFileAttribute {
};
class UnixFileInfo {
    friend class UnixDirectoryIterator;
    protected:
    StringView fileName{};
    Path fullPath{};
    public:
    constexpr UnixFileInfo() = default;
    const auto& path() const { return fullPath; }
    const auto& filename() const { return fileName; }
};
using UnixDirIterHandle = glob_t*;
bool remove_filespec(String& p);
class UnixDirectoryIterator {
    friend class UnixDirectoryIterate;
    UnixDirIterHandle m_hdl;
    const Path& m_path;
    UnixFileInfo& m_info;
    size_t m_index;
    protected:
    UnixDirectoryIterator(const Path& path, UnixFileInfo& info);
    UnixDirectoryIterator(const Path& path, UnixDirIterHandle hdl, UnixFileInfo& info);
    public:
    UnixDirectoryIterator(const UnixDirectoryIterator&)             = delete;
    UnixDirectoryIterator& operator=(const UnixDirectoryIterator&) = delete;
    UnixDirectoryIterator(UnixDirectoryIterator&&) noexcept;
    UnixDirectoryIterator& operator=(UnixDirectoryIterator&&) noexcept = delete;
    bool operator==(const UnixDirectoryIterator& other) const;
    bool operator!=(const UnixDirectoryIterator& other) const;
    const UnixFileInfo& operator*() const;
    UnixDirectoryIterator& operator++();
    ~UnixDirectoryIterator();
};
class UnixDirectoryIterate {
    mutable UnixFileInfo m_info{};
    Path m_path;
    mutable glob_t m_glob_result{};
    public:
    UnixDirectoryIterate(Path path) : m_path(move(path)){};
    auto begin() const { return UnixDirectoryIterator{ m_path, &m_glob_result, m_info }; }
    auto end() const { return UnixDirectoryIterator{ m_path, m_info }; }
};
}    // namespace ARLib
#endif
