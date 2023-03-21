#pragma once
#include "Compat.h"
#include "GeneratedEnums/OpenFileMode.h"
#include "HashBase.h"
#include "Optional.h"
#include "PrintInfo.h"
#include "Result.h"
#include "String.h"
#include "StringView.h"
#include "Variant.h"
#include "Types.h"
#include "arlib_osapi.h"
#include "cstdio_compat.h"
#include "FileSystem.h"
namespace ARLib {
class FileError : public ErrorBase {
    Path m_filename{};
    String m_error{};

    public:
    FileError() = default;
    FileError(String error, Path filename) : m_filename(move(filename)), m_error(move(error)) {}
    const Path& filename() const { return m_filename; }
    const String& error_string() const override { return m_error; }
};
class File {
    constexpr static inline size_t LINELENGTH_MAX = 1024;
    FILE* m_ptr                                   = nullptr;
    Path m_filename;
    OpenFileMode m_mode;

    using WriteResult = Result<size_t, FileError>;
    using ReadResult = Result<String, FileError>;
    friend Hash<File>;

    public:
    explicit File(Path filepath) : m_filename(move(filepath)), m_mode(OpenFileMode::None) {}
    explicit File(FsString filename) : m_filename(move(filename)), m_mode(OpenFileMode::None) {}
    explicit File(NonFsString filename) : m_filename(convert_from_non_fs_to_fs(filename)), m_mode(OpenFileMode::None) {}
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
        m_mode = mode;
        switch (mode) {
            case OpenFileMode::Read:
                m_ptr = fopen(m_filename.string().data(), "r");
                break;
            case OpenFileMode::Write:
                m_ptr = fopen(m_filename.string().data(), "w");
                break;
            case OpenFileMode::ReadWrite:
                m_ptr = fopen(m_filename.string().data(), "w+");
                break;
            case OpenFileMode::Append:
                m_ptr = fopen(m_filename.string().data(), "a+");
                break;
            default:
                break;
        }
        if (!m_ptr) {
            return FileError{ last_error(), m_filename };
        } else {
            return {};
        }
    }
    WriteResult write(const String& str) {
        if (m_mode != OpenFileMode::Write && m_mode != OpenFileMode::Append) {
            return FileError{ "Can't write to a file opened in read-only mode"_s, m_filename };
        }
        auto len = ARLib::fwrite(str.data(), str.size(), sizeof(char), m_ptr);
        if (len != str.size()) { return FileError{ "Failed to write the whole string into the file"_s, m_filename }; }
        return len;
    }
    ReadResult read_n(size_t count) {
        if (m_mode != OpenFileMode::Read) {
            return FileError{ "Can't read from a file not open in read mode"_s, m_filename };
        }
        String line{ count, '\0' };
        line.set_size(ARLib::fread(line.rawptr(), count, sizeof(char), m_ptr));
        if (line.size() != count) return FileError{ "Couldn't read requested size"_s, m_filename };
        return line;
    }
    ReadResult read_line() {
        if (m_mode != OpenFileMode::Read) {
            return FileError{ "Can't read from a file not open in read mode"_s, m_filename };
        }
        String line{ LINELENGTH_MAX, '\0' };
        line.set_size(ARLib::fread(line.rawptr(), sizeof(char), LINELENGTH_MAX, m_ptr));
        return line;
    }
    ReadResult read_all() {
        if (m_mode != OpenFileMode::Read) {
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
    size_t size() const {
        HARD_ASSERT(m_ptr != nullptr, "File has to be open to ask for the size");
        return filesize(m_ptr);
    }
    ~File() {
        if (m_ptr) ARLib::fclose(m_ptr);
    }
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
