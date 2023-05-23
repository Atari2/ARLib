#pragma once
#define FILESYSTEM_INCLUDED__
#include "XNative/fs/xnative_fs_merge.hpp"
#include "Chrono.hpp"
namespace ARLib {
class FileInfo {
    NativeFileInfo m_info;
    public:
    FileInfo(const Path& path) : m_info(path) {}
    FileInfo(NativeFileInfo&& info) : m_info(move(info)) {}
    FileInfo(const NativeFileInfo& info) : m_info(info) {}
    FileInfo(const FileInfo& other) = default;
    FileInfo(FileInfo&& other)  noexcept : m_info(move(other.m_info)) {}
    FileInfo& operator=(const FileInfo& other) = default;
    FileInfo& operator=(FileInfo&& other)  noexcept {
        m_info = move(other.m_info);
        return *this;
    }
    const Path& path() const;
    const FsStringView& filename() const;
    bool is_directory() const;
    bool is_file() const;
    size_t filesize() const;
    Instant last_access() const;
    Instant last_modification() const;
};
class DirectoryIterator {
    friend class DirectoryIterate;
    NativeDirectoryIterator m_native_iter;
    DirectoryIterator(NativeDirectoryIterator&& iter) : m_native_iter(move(iter)) {}
    public:
    DirectoryIterator(const DirectoryIterator&)                = delete;
    DirectoryIterator& operator=(const DirectoryIterator&)     = delete;
    DirectoryIterator(DirectoryIterator&&) noexcept            = default;
    DirectoryIterator& operator=(DirectoryIterator&&) noexcept = delete;
    bool operator==(const DirectoryIterator& other) const { return m_native_iter == other.m_native_iter; }
    bool operator!=(const DirectoryIterator& other) const { return m_native_iter != other.m_native_iter; }
    FileInfo operator*() const { return FileInfo{ *m_native_iter }; }
    FileInfo operator*() { return FileInfo{ *m_native_iter }; }
    DirectoryIterator& operator++() {
        ++m_native_iter;
        return *this;
    }
    ~DirectoryIterator() = default;
};
class DirectoryIterate {
    NativeDirectoryIterate m_native;
    public:
    DirectoryIterate(Path path, bool recurse = false) : m_native(move(path), recurse){};
    auto begin() const { return DirectoryIterator{ m_native.begin() }; }
    auto end() const { return DirectoryIterator{ m_native.end() }; }
};
}    // namespace ARLib
