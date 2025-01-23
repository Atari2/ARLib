#pragma once
#include "Result.hpp"
#include "Span.hpp"
#include "String.hpp"
#include "File.hpp"
#include "MaybeOwned.hpp"
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
    virtual ~BaseStream()       = default;
};
struct CharacterStream : public BaseStream {
    class LinesIterator {
        MaybeOwned<CharacterStream> m_stream;
        bool m_eof_reached{ false };
        bool m_end{ false };
        String m_current_line{};
        public:
        LinesIterator(MaybeOwned<CharacterStream> stream, bool end = false);
        String operator*();
        LinesIterator& operator++();
        LinesIterator operator++(int);
        bool operator==(const LinesIterator& other) const;
        bool operator!=(const LinesIterator& other) const;
    };
    class Lines {
        MaybeOwned<CharacterStream> m_stream;
        public:
        template <DerivedFrom<CharacterStream> U>
        Lines(U& stream) : m_stream{ MaybeOwned<CharacterStream>::template lended<U>(stream) } {
            if (auto& fs = *m_stream; !fs.is_open()) { fs.open(); }
        }
        template <DerivedFrom<CharacterStream> U>
        Lines(U&& stream) : m_stream{ MaybeOwned<CharacterStream>::template owned<U>(Forward<U>(stream)) } {
            if (auto& fs = *m_stream; !fs.is_open()) { fs.open(); }
        }
        LinesIterator begin();
        LinesIterator end();
        LinesIterator begin() const;
        LinesIterator end() const;
    };
    virtual Lines lines() &  = 0;
    virtual Lines lines() && = 0;
    virtual DiscardResult<FileError> open() { return DefaultOk{}; }
    virtual Result<size_t> write_string(StringView buffer) = 0;
    virtual Result<String> read_string()                   = 0;
    virtual Result<String> read_line(bool& eof_reached)    = 0;
    virtual bool is_open() const                           = 0;
    virtual bool operator==(const CharacterStream& other) const { return this == &other; }
    virtual ~CharacterStream() = default;
};
class FileStream : public CharacterStream {
    protected:
    File m_file;
    public:
    FileStream(File&& file) : m_file{ move(file) } {}
    FileStream(Path filepath) : m_file{ move(filepath) } {}
    DiscardResult<FileError> open() override;
    Result<size_t> write(Span<const uint8_t> buffer) override;
    Result<Vector<uint8_t>> read(size_t n) override;
    Result<Vector<uint8_t>> read() override;
    Result<size_t> write_string(StringView buffer) override;
    Result<String> read_string() override;
    Result<String> read_line(bool& eof_reached) override;
    Lines lines() & override { return Lines{ *this }; }
    Lines lines() && override { return Lines{ move(*this) }; }
    size_t pos() const override;
    size_t seek(size_t) override;
    bool operator==(const FileStream& other) const;
    bool operator==(const CharacterStream& other) const override {
        if (const auto* fs = dynamic_cast<const FileStream*>(&other); fs != nullptr) { return *this == *fs; }
        return false;
    }
    bool is_open() const override { return m_file.is_open(); }
    virtual ~FileStream() = default;
};
class BufferedFileStream : public FileStream {
    size_t m_buffer_capacity;
    String m_buffer;
    public:
    BufferedFileStream(Path filepath, size_t buffer_size = get_page_size()) :
        FileStream{ move(filepath) }, m_buffer_capacity{ buffer_size }, m_buffer{} {
        m_buffer.reserve(buffer_size);
    }
    BufferedFileStream(File&& file, size_t buffer_size = get_page_size()) :
        FileStream{ Forward<File>(file) }, m_buffer_capacity{ buffer_size }, m_buffer{} {
        m_buffer.reserve(buffer_size);
    }
    Result<size_t> write(Span<const uint8_t> buffer) override;
    Result<Vector<uint8_t>> read(size_t n) override;
    Result<Vector<uint8_t>> read() override;
    Result<size_t> write_string(StringView buffer) override;
    Result<String> read_string() override;
    Result<String> read_line(bool& eof_reached) override;
    size_t pos() const override;
    size_t seek(size_t) override;
    void flush();
    virtual ~BufferedFileStream();
};
class StringStream : public CharacterStream {
    size_t m_pos{ 0 };
    String m_buffer;
    public:
    StringStream() = default;
    StringStream(String buffer) : m_buffer{ move(buffer) } {}
    Result<size_t> write(Span<const uint8_t> buffer) override;
    Result<Vector<uint8_t>> read(size_t n) override;
    Result<Vector<uint8_t>> read() override;
    Result<size_t> write_string(StringView buffer) override;
    Result<String> read_string() override;
    Result<String> read_line(bool& eof_reached) override;
    Lines lines() & override { return Lines{ *this }; }
    Lines lines() && override { return Lines{ move(*this) }; }
    size_t pos() const override { return m_pos; }
    size_t seek(size_t pos) override {
        m_pos = pos;
        return m_pos;
    }
    String str() const { return m_buffer; }
    bool is_open() const override { return true; }
    virtual ~StringStream() = default;
};
}    // namespace ARLib