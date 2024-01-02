#pragma once
#include "Result.hpp"
#include "Span.hpp"
#include "String.hpp"
#include "File.hpp"
namespace ARLib {
struct BaseStream {
    // write bytes
    virtual Result<size_t> write(Span<const uint8_t> buffer) = 0;
    // read n bytes
    virtual Result<Vector<uint8_t>> read(size_t n) = 0;
    // read until end
    virtual Result<Vector<uint8_t>> read() = 0;
    // get current position in stream
    virtual size_t pos() const = 0;
    // seek to position in stream
    virtual size_t seek(size_t) = 0;
    virtual ~BaseStream()      = default;
};
struct CharacterStream : public BaseStream {
    virtual DiscardResult<FileError> open() { return DefaultOk{}; }
    virtual Result<size_t> write_string(StringView buffer) = 0;
    virtual Result<String> read_string()                   = 0;
    virtual Result<String> read_line(bool& eof_reached)    = 0;
    virtual ~CharacterStream()                             = default;
};
class FileStream : public CharacterStream {
    File m_file;
    public:
    FileStream(Path filepath) : m_file{ move(filepath) } {}
    DiscardResult<FileError> open() override;
    Result<size_t> write(Span<const uint8_t> buffer) override;
    Result<Vector<uint8_t>> read(size_t n) override;
    Result<Vector<uint8_t>> read() override;
    Result<size_t> write_string(StringView buffer) override;
    Result<String> read_string() override;
    Result<String> read_line(bool& eof_reached) override;
    size_t pos() const override;
    size_t seek(size_t) override;
    virtual ~FileStream() = default;
};
class StringStream : public CharacterStream {
    size_t m_pos{ 0 };
    String m_buffer;
    public:
    StringStream(String buffer) : m_buffer{ move(buffer) } {}
    Result<size_t> write(Span<const uint8_t> buffer) override;
    Result<Vector<uint8_t>> read(size_t n) override;
    Result<Vector<uint8_t>> read() override;
    Result<size_t> write_string(StringView buffer) override;
    Result<String> read_string() override;
    Result<String> read_line(bool& eof_reached) override;
    size_t pos() const override { return m_pos; }
    size_t seek(size_t pos) override {
        m_pos = pos;
        return m_pos;
    }
    String str() const { return m_buffer; }
    virtual ~StringStream() = default;
};
}    // namespace ARLib