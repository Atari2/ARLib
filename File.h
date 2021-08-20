#pragma once
#include "Compat.h"
#include "GeneratedEnums/OpenFileMode.h"
#include "Optional.h"
#include "Result.h"
#include "String.h"
#include "StringView.h"
#include "Types.h"
#include "cstdio_compat.h"

namespace ARLib {
    struct OpenFileError {
        OpenFileError() = default;
        static constexpr inline StringView error = "File couldn't be opened"_sv;
    };

    struct ReadFileError {
        ReadFileError() = default;
        static constexpr inline StringView error = "File couldn't be read correctly"_sv;
    };

    struct WriteFileError {
        WriteFileError() = default;
        static constexpr inline StringView error = "File couldn't be written correctly"_sv;
    };

    class File {
        constexpr inline static size_t LINELENGTH_MAX = 1024;
        FILE* m_ptr = nullptr;
        String m_filename;
        OpenFileMode m_mode;

        public:
        explicit File(String filename) : m_filename(move(filename)), m_mode(OpenFileMode::None) {}

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
                return OpenFileError{};
            } else {
                return {};
            }
        }

        Result<size_t, WriteFileError> write(const String& str) {
            if (m_mode != OpenFileMode::Write) { return Result<size_t, WriteFileError>::from_error(); }
            auto len = fwrite(str.data(), str.size(), sizeof(char), m_ptr);
            if (len != str.size()) { return Result<size_t, WriteFileError>::from_error(); }
            return Result<size_t, WriteFileError>::from_ok(len);
        }

        Result<String, ReadFileError> read_n(size_t count) {
            if (m_mode != OpenFileMode::Read) { return Result<String, ReadFileError>::from_error(); }
            String line{count, '\0'};
            line.set_size(fread(line.rawptr(), count, sizeof(char), m_ptr));
            if (line.size() != count) return Result<String, ReadFileError>::from_error();
            return Result<String, ReadFileError>{Forward<String>(line)};
        }

        Result<String, ReadFileError> read_line() {
            if (m_mode != OpenFileMode::Read) { return Result<String, ReadFileError>::from_error(); }
            String line{LINELENGTH_MAX, '\0'};
            line.set_size(fread(line.rawptr(), sizeof(char), LINELENGTH_MAX, m_ptr));
            return Result<String, ReadFileError>{Forward<String>(line)};
        }

        Result<String, ReadFileError> read_all() {
            if (m_mode != OpenFileMode::Read) { return Result<String, ReadFileError>::from_error(); }
            String line{};
            fseek(m_ptr, 0, SEEK_END);
            auto len = ftell(m_ptr);
            fseek(m_ptr, 0, SEEK_SET);
            line.reserve(len);
            line.set_size(fread(line.rawptr(), sizeof(char), len, m_ptr));
#ifndef WINDOWS
            if (line.size() != len) { return Result<String, ReadFileError>::from_error(); }
#endif
            return Result<String, ReadFileError>{Forward<String>(line)};
        }
        ~File() { fclose(m_ptr); }
    };
} // namespace ARLib

using ARLib::File;
using ARLib::OpenFileMode;