#pragma once

#include "Path.hpp"
#include "Vector.hpp"
#include "Stream.hpp"
#include "Result.hpp"
#include "SharedPtr.hpp"
namespace ARLib {
struct CSVErrorInfo {
    String error_string;
    size_t error_offset;
};
class CSVParseError : public ErrorBase {
    CSVErrorInfo m_info;

    public:
    CSVParseError() = default;
    CSVParseError(StringView error, size_t offset) : m_info{ error.str(), offset } {};
    CSVParseError(CSVErrorInfo info) : m_info(move(info)){};
    const CSVErrorInfo& info() const { return m_info; }
    const String& message() const { return m_info.error_string; }
    StringView error_string() const { return m_info.error_string; }
    size_t offset() const { return m_info.error_offset; }
    virtual bool is_eof() const { return false; };
};
class CSVEndOfFileError : public CSVParseError {
    public:
    CSVEndOfFileError() : CSVParseError{ "End-of-file reached"_s, npos_ } {}
    bool is_eof() const override { return true; }
};
class CSVRow;
class CSVParser;
using CSVRowResult = Result<Pair<CSVRow, String>, CSVParseError>;
using CSVResult    = Result<CSVRow, CSVParseError>;
using CSVHeader    = Vector<String>;
class CSVRow {
    friend class CSVParser;
    friend struct PrintInfo<CSVRow>;
    Vector<String> m_row;
    WeakPtr<Optional<CSVHeader>> m_header;
    static CSVRowResult parse_row(CharacterStream* line, String&& leftover, char sep, const CSVParser& parser);
    CSVRow(Vector<String>&& row, WeakPtr<Optional<CSVHeader>>&& ptr) : m_row(move(row)), m_header{ move(ptr) } {}
    public:
    const String& operator[](size_t index) const { return m_row[index]; }
    Result<const String&> operator[](StringView name) const {
        if (m_header->empty()) { return Error{ "CSVRow does not have a header"_s }; }
        const auto idx = find_if(m_header->value(), [&name](const auto& f) { return f.view() == name; });
        if (idx == npos_) return Error{ "Header of row does not contain this key"_s };
        return m_row[idx];
    }
    const String& field(size_t index) const { return m_row[index]; }
    Result<const String&> field(StringView name) const { return this->operator[](name); }
};
class CSVParser {
    SharedPtr<Optional<CSVHeader>> m_header;
    UniquePtr<CharacterStream> m_stream;
    char m_separator;
    bool m_has_header;
    String m_leftover;

    public:
    CSVParser(Path p) :
        m_header{ Optional<CSVHeader>{} }, m_stream{ new FileStream{ move(p) } }, m_separator{ ',' },
        m_has_header{ false },
        m_leftover{} {}
    CSVParser(String p) :
    m_header{ Optional<CSVHeader>{} }, m_stream{ new StringStream{ move(p) } }, m_separator{ ',' },
    m_has_header{ false }, m_leftover{} {}
    DiscardResult<FileError> open() { return m_stream->open(); }
    void with_header(bool has_header) { m_has_header = move(has_header); }
    void with_separator(char separator) { m_separator = separator; }
    CSVResult read_row();
    Result<Vector<CSVResult>, CSVParseError> read_all();
    const auto& header() const { return m_header; }
};
template <>
struct PrintInfo<CSVRow> {
    const CSVRow& m_row;
    PrintInfo(const CSVRow& row) : m_row{ row } {}
    String repr() const {
        String output{};
        if (m_row.m_header->has_value()) {
            const auto& header = m_row.m_header->value();
            output += "Header: "_s + print_conditional(header);
        }
        if (!output.is_empty()) { output += '\n'; }
        output += "Row: "_s + print_conditional(m_row.m_row);
        return output;
    }
};
}    // namespace ARLib