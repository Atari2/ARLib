#pragma once
#include "Compat.h"
#include "GeneratedEnums/OpenFileMode.h"
#include "HashBase.h"
#include "Optional.h"
#include "PrintInfo.h"
#include "Result.h"
#include "String.h"
#include "StringView.h"
#include "Types.h"
#include "arlib_osapi.h"
#include "cstdio_compat.h"

namespace ARLib {
    class OpenFileError {
        String m_filename{};
        String m_error{};

        public:
        OpenFileError() = default;
        OpenFileError(String error, String filename) : m_filename(move(filename)), m_error(move(error)) {}
        const String& filename() const { return m_filename; }
        const String& error() const { return m_error; }
    };

    struct ReadFileError {
        ReadFileError() = default;
        static constexpr inline StringView error = "File couldn't be read correctly"_sv;
    };

    struct WriteFileError {
        WriteFileError() = default;
        static constexpr inline StringView error = "File couldn't be written correctly"_sv;
    };

    template <>
    struct PrintInfo<ReadFileError> {
        const ReadFileError& m_error;
        explicit PrintInfo(const ReadFileError& error) : m_error(error) {}
        String repr() const { return String{m_error.error}; }
    };
    template <>
    struct PrintInfo<OpenFileError> {
        const OpenFileError& m_error;
        explicit PrintInfo(const OpenFileError& error) : m_error(error) {}
        String repr() const { return m_error.error() + " - Filename: "_s + m_error.filename(); }
    };
    template <>
    struct PrintInfo<WriteFileError> {
        const WriteFileError& m_error;
        explicit PrintInfo(const WriteFileError& error) : m_error(error) {}
        String repr() const { return String{m_error.error}; }
    };

    class File {
        constexpr inline static size_t LINELENGTH_MAX = 1024;
        FILE* m_ptr = nullptr;
        String m_filename;
        OpenFileMode m_mode;

        using WriteResult = Result<size_t, WriteFileError>;
        using ReadResult = Result<String, ReadFileError>;
        friend Hash<File>;

        public:
        explicit File(String filename) : m_filename(move(filename)), m_mode(OpenFileMode::None) {}

        OpenFileMode mode() const { return m_mode; }
        const String& name() const { return m_filename; }

        void remove() {
            if (m_mode != OpenFileMode::None) { ARLib::fclose(m_ptr); }
            ARLib::remove(m_filename.data());
            m_filename = ""_s;
            m_mode = OpenFileMode::None;
        }

        static void remove(StringView name) { ARLib::remove(name.data()); }

        void rename(StringView new_name) {
            if (m_mode != OpenFileMode::None) {
                long pl = static_cast<long>(ARLib::ftell(m_ptr));
                ARLib::fclose(m_ptr);
                ARLib::rename(m_filename.data(), new_name.data());
                m_filename = new_name.extract_string();
                open(m_mode);
                ARLib::fseek(m_ptr, pl, SEEK_SET);
            } else {
                ARLib::rename(m_filename.data(), new_name.data());
            }
        }

        static void rename(StringView old_name, StringView new_name) {
            ARLib::rename(old_name.data(), new_name.data());
        }

        Optional<OpenFileError> open(OpenFileMode mode) {
            m_mode = mode;
            switch (mode) {
            case OpenFileMode::Read:
                m_ptr = fopen(m_filename.data(), "r");
                break;
            case OpenFileMode::Write:
                m_ptr = fopen(m_filename.data(), "w");
                break;
            case OpenFileMode::ReadWrite:
                m_ptr = fopen(m_filename.data(), "w+");
                break;
            default:
                break;
            }
            if (!m_ptr) {
                return OpenFileError{last_error(), m_filename};
            } else {
                return {};
            }
        }

        WriteResult write(const String& str) {
            if (m_mode != OpenFileMode::Write) { return WriteResult::from_error(); }
            auto len = ARLib::fwrite(str.data(), str.size(), sizeof(char), m_ptr);
            if (len != str.size()) { return WriteResult::from_error(); }
            return WriteResult::from_ok(len);
        }

        ReadResult read_n(size_t count) {
            if (m_mode != OpenFileMode::Read) { return ReadResult::from_error(); }
            String line{count, '\0'};
            line.set_size(ARLib::fread(line.rawptr(), count, sizeof(char), m_ptr));
            if (line.size() != count) return ReadResult::from_error();
            return ReadResult{Forward<String>(line)};
        }

        ReadResult read_line() {
            if (m_mode != OpenFileMode::Read) { return ReadResult::from_error(); }
            String line{LINELENGTH_MAX, '\0'};
            line.set_size(ARLib::fread(line.rawptr(), sizeof(char), LINELENGTH_MAX, m_ptr));
            return ReadResult{Forward<String>(line)};
        }

        ReadResult read_all() {
            if (m_mode != OpenFileMode::Read) { return ReadResult::from_error(); }
            String line{};
            ARLib::fseek(m_ptr, 0, SEEK_END);
            auto len = ARLib::ftell(m_ptr);
            ARLib::fseek(m_ptr, 0, SEEK_SET);
            line.reserve(len);
            line.set_size(ARLib::fread(line.rawptr(), sizeof(char), len, m_ptr));
#ifndef WINDOWS
            if (line.size() != len) { return ReadResult::from_error(); }
#endif
            return ReadResult{Forward<String>(line)};
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
        String repr() const { return "File { "_s + m_file.name() + " }"_s; }
    };

} // namespace ARLib
