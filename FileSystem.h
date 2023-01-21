#pragma once
#define FILESYSTEM_INCLUDED__
#include "XNative/fs/xnative_fs_merge.h"
namespace ARLib {
class FileInfo {
    const NativeFileInfo& m_info;
    public:
    FileInfo(const NativeFileInfo& info) : m_info(info) {}
    const Path& path() const;
    const FsStringView& filename() const;
};
class DirectoryIterator {
    friend class DirectoryIterate;
    NativeDirectoryIterator m_native_iter;
    FileInfo m_info;
    DirectoryIterator(NativeDirectoryIterator&& iter) : m_native_iter(move(iter)), m_info(*m_native_iter) {}
    public:
    DirectoryIterator(const DirectoryIterator&)                = delete;
    DirectoryIterator& operator=(const DirectoryIterator&)     = delete;
    DirectoryIterator(DirectoryIterator&&) noexcept            = default;
    DirectoryIterator& operator=(DirectoryIterator&&) noexcept = delete;
    bool operator==(const DirectoryIterator& other) const { return m_native_iter == other.m_native_iter; }
    bool operator!=(const DirectoryIterator& other) const { return m_native_iter != other.m_native_iter; }
    const FileInfo& operator*() const { return m_info; }
    DirectoryIterator& operator++() {
        ++m_native_iter;
        return *this;
    }
    ~DirectoryIterator() = default;
};
class DirectoryIterate {
    NativeDirectoryIterate m_native;
    public:
    DirectoryIterate(Path path) : m_native(move(path)){};
    auto begin() const { return DirectoryIterator{ m_native.begin() }; }
    auto end() const { return DirectoryIterator{ m_native.end() }; }
};
}    // namespace ARLib
