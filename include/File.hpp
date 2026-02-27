#pragma once
#include "Compat.hpp"
#include "GeneratedEnums/OpenFileMode.hpp"
#include "HashBase.hpp"
#include "Optional.hpp"
#include "PrintInfo.hpp"
#include "Result.hpp"
#include "String.hpp"
#include "StringView.hpp"
#include "Variant.hpp"
#include "Types.hpp"
#include "arlib_osapi.hpp"
#include "cstdio_compat.hpp"
#include "FileSystem.hpp"
namespace ARLib {
class FileError : public ErrorBase {
    Path m_filename{};
    String m_error{};

    public:
    FileError() = default;
    FileError(String error, Path filename) : m_filename(move(filename)), m_error(move(error)) {}
    const Path& filename() const { return m_filename; }
    StringView error_string() const override { return m_error; }
};
class File {
    constexpr static inline size_t LINELENGTH_MAX = 1024;
    FILE* m_ptr                                   = nullptr;
    Path m_filename;
    OpenFileMode m_mode;

    using WriteResult = Result<size_t, FileError>;
    using ReadResult  = Result<String, FileError>;
    friend Hash<File>;

    public:
    explicit File(Path filepath) : m_filename(move(filepath)), m_mode(OpenFileMode::None) {}
    explicit File(FsString filename) : m_filename(move(filename)), m_mode(OpenFileMode::None) {}
    explicit File(NonFsString filename) : m_filename(convert_from_non_fs_to_fs(filename)), m_mode(OpenFileMode::None) {}
    File(const File& other)            = delete;
    File& operator=(const File& other) = delete;
    File(File&& other) noexcept : m_ptr(other.m_ptr), m_filename(move(other.m_filename)), m_mode(other.m_mode) {
        other.m_ptr  = nullptr;
        other.m_mode = OpenFileMode::None;
    }
    File& operator=(File&& other) noexcept {
        if (this != &other) {
            if (m_mode != OpenFileMode::None) { ARLib::fclose(m_ptr); }
            m_ptr        = other.m_ptr;
            m_filename   = move(other.m_filename);
            m_mode       = other.m_mode;
            other.m_ptr  = nullptr;
            other.m_mode = OpenFileMode::None;
        }
        return *this;
    }
    OpenFileMode mode() const { return m_mode; }
    const auto& name() const { return m_filename; }
    void remove() {
        if (m_mode != OpenFileMode::None) { ARLib::fclose(m_ptr); }
        ARLib::remove(m_filename.string().data());
        m_filename = FsString{};
        m_mode     = OpenFileMode::None;
    }
    static void remove(const Path& path) { ARLib::remove(path.string().data()); }
    void rename(const Path& new_name) {
        if (m_mode != OpenFileMode::None) {
            long pl = static_cast<long>(ARLib::ftell(m_ptr));
            ARLib::fclose(m_ptr);
            ARLib::rename(m_filename.string().data(), new_name.string().data());
            m_filename = new_name;
            open(m_mode);
            ARLib::fseek(m_ptr, pl, SEEK_SET);
        } else {
            ARLib::rename(m_filename.string().data(), new_name.string().data());
        }
    }
    static void rename(const Path& old, const Path& new_) { ARLib::rename(old.string().data(), new_.string().data()); }
    DiscardResult<FileError> open(OpenFileMode mode) {
        if (m_ptr != nullptr) { return FileError{ "File is already open"_s, m_filename }; }
        m_mode           = mode;
        bool is_readable = (mode & OpenFileMode::Read) != OpenFileMode::None;
        bool is_writable = (mode & OpenFileMode::Write) != OpenFileMode::None;
        bool is_append   = (mode & OpenFileMode::Append) != OpenFileMode::None;

        if (is_writable && is_append) { return FileError{ "Append mode implies write mode"_s, m_filename }; }

        if (is_readable && is_writable) {
            m_ptr = fopen(m_filename.string().data(), "w+");
        } else if (is_append && is_readable) {
            m_ptr = fopen(m_filename.string().data(), "a+");
        } else if (is_readable) {
            m_ptr = fopen(m_filename.string().data(), "r");
        } else if (is_writable) {
            m_ptr = fopen(m_filename.string().data(), "w");
        } else if (is_append) {
            m_ptr = fopen(m_filename.string().data(), "a");
        } else {
            return FileError{ "Invalid open file mode"_s, m_filename };
        }
        if (!m_ptr) {
            return FileError{ last_error(), m_filename };
        } else {
            return {};
        }
    }
    WriteResult write(StringView str) {
        bool is_writable = (m_mode & OpenFileMode::Write) != OpenFileMode::None;
        bool is_append   = (m_mode & OpenFileMode::Append) != OpenFileMode::None;
        if (!is_writable && !is_append) {
            return FileError{ "Can't write to a file opened in read-only mode"_s, m_filename };
        }
        auto len = ARLib::fwrite(str.data(), sizeof(char), str.size(), m_ptr);
        if (len != str.size()) { return FileError{ "Failed to write the whole string into the file"_s, m_filename }; }
        return len;
    }
    size_t pos() const { return ARLib::ftell(m_ptr); }
    size_t seek(size_t pos) { return static_cast<size_t>(ARLib::fseek(m_ptr, static_cast<long>(pos), SEEK_SET)); }
    WriteResult write(char c) { return write(StringView{ &c, 1 }); }
    WriteResult write(const String& str) { return write(str.view()); }
    ReadResult read_n(size_t count) {
        if ((m_mode & OpenFileMode::Read) == OpenFileMode::None) {
            return FileError{ "Can't read from a file not open in read mode"_s, m_filename };
        }
        String line{ count, '\0' };
        line.set_size(ARLib::fread(line.rawptr(), sizeof(char), count, m_ptr));
        if (line.size() != count) return FileError{ "Couldn't read requested size"_s, m_filename };
        return line;
    }
    ReadResult read_line(bool& eof_reached);
    ReadResult read_all() {
        if ((m_mode & OpenFileMode::Read) == OpenFileMode::None) {
            return FileError{ "Can't read from a file not open in read mode"_s, m_filename };
        }
        String line{};
        ARLib::fseek(m_ptr, 0, SEEK_END);
        auto len = ARLib::ftell(m_ptr);
        ARLib::fseek(m_ptr, 0, SEEK_SET);
        line.reserve(len);
        line.set_size(ARLib::fread(line.rawptr(), sizeof(char), len, m_ptr));
#ifndef WINDOWS
        if (line.size() != len) { return FileError{ "Failed to read the full file"_s, m_filename }; }
#endif
        return line;
    }
    template <typename T>
    requires IsAnyOfV<T, FsString, NonFsString, Path>
    static ReadResult read_all(const T& filename) {
        File f{ filename };
        TRY(f.open(OpenFileMode::Read));
        TRY_RET(f.read_all());
    }
    template <typename T>
    requires IsAnyOfV<T, FsString, NonFsString, Path>
    static WriteResult write_all(const T& filename, const String& str) {
        File f{ filename };
        TRY(f.open(OpenFileMode::Write));
        TRY_RET(f.write(str));
    }
    size_t size() const {
        HARD_ASSERT(m_ptr != nullptr, "File has to be open to ask for the size");
        return filesize(m_ptr);
    }
    void close() {
        if (m_ptr) ARLib::fclose(m_ptr);
    }
    bool is_open() const { return m_ptr != nullptr; }
    bool operator==(const File& other) const { return m_filename == other.m_filename; }
    ~File() { close(); }
};
template <>
struct Hash<File> {
    [[nodiscard]] size_t operator()(const File& key) const noexcept { return hash_representation(key.m_ptr); }
};
template <>
struct PrintInfo<File> {
    const File& m_file;
    explicit PrintInfo(const File& file) : m_file(file) {}
    String repr() const { return "File { "_s + m_file.name().narrow() + " }"_s; }
};
}    // namespace ARLib
