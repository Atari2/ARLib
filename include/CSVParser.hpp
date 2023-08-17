#pragma once

#include "Path.hpp"
#include "Vector.hpp"
#include "File.hpp"
#include "Result.hpp"
namespace ARLib {
class CSVHeader {
    friend class CSVParser;
    Vector<String> m_row;
    static Result<CSVHeader> parse_header(String&& line);
    CSVHeader(Vector<String>&& row) : m_row(move(row)) {}
};
class CSVRow {
    friend class CSVParser;
    Vector<String> m_row;
    static Result<CSVRow> parse_row(String&& line);
    CSVRow(Vector<String>&& row) : m_row(move(row)) {}
};
class CSVParser {
    Vector<CSVRow> m_rows;
    Optional<CSVHeader> m_header;
    File m_file;
    char m_separator;
    bool m_has_header;
    bool m_read_header;

    using CSVRH = Variant<CSVRow, CSVHeader>;

    public:
    CSVParser(Path p) :
        m_rows{}, m_header{}, m_file{ move(p) }, m_separator{ ',' }, m_has_header{ false }, m_read_header{ false } {}
    DiscardResult<FileError> open();
    void with_header(bool has_header) { m_has_header = move(has_header); }
    void with_separator(char separator) { m_separator = separator; }
    Result<CSVRH> read_row();
    Result<Vector<CSVRH>> read_all();
};
}    // namespace ARLib