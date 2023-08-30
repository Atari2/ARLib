#pragma once

#include "Path.hpp"
#include "Vector.hpp"
#include "File.hpp"
#include "Result.hpp"
namespace ARLib {
struct CSVErrorInfo {
    String error_string;
    size_t error_offset;
};
class CSVParseError : public ErrorBase {
    CSVErrorInfo m_info;

    public:
    CSVParseError() = default;
    CSVParseError(String error, size_t offset) : m_info{ move(error), offset } {};
    CSVParseError(CSVErrorInfo info) : m_info(move(info)){};
    const CSVErrorInfo& info() const { return m_info; }
    const String& message() const { return m_info.error_string; }
    const String& error_string() const { return m_info.error_string; }
    size_t offset() const { return m_info.error_offset; }
};
class CSVRow;
class CSVHeader;
using CSVRH           = Variant<CSVRow, CSVHeader>;
using CSVHeaderResult = Result<CSVHeader, CSVParseError>;
using CSVRowResult    = Result<CSVRow, CSVParseError>;
using CSVResult       = Result<CSVRH, CSVParseError>;
class CSVHeader {
    friend class CSVParser;
    Vector<String> m_row;
    static CSVHeaderResult parse_header(File& line);
    CSVHeader(Vector<String>&& row) : m_row(move(row)) {}
};
class CSVRow {
    friend class CSVParser;
    friend struct PrintInfo<CSVRow>;
    Vector<String> m_row;
    static CSVRowResult parse_row(File& line);
    CSVRow(Vector<String>&& row) : m_row(move(row)) {}
};
class CSVParser {
    Vector<CSVRow> m_rows;
    Optional<CSVHeader> m_header;
    File m_file;
    char m_separator;
    bool m_has_header;
    bool m_read_header;

    public:
    CSVParser(Path p) :
        m_rows{}, m_header{}, m_file{ move(p) }, m_separator{ ',' }, m_has_header{ false }, m_read_header{ false } {}
    DiscardResult<FileError> open();
    void with_header(bool has_header) { m_has_header = move(has_header); }
    void with_separator(char separator) { m_separator = separator; }
    CSVResult read_row();
    Result<Vector<CSVRH>, CSVParseError> read_all();
};
template <>
struct PrintInfo<CSVRow> {
    const CSVRow& m_row;
    PrintInfo(const CSVRow& row) : m_row{ row } {}
    String repr() const { return print_conditional(m_row.m_row); }
};
}    // namespace ARLib