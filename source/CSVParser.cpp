#include "CSVParser.hpp"
namespace ARLib {
DiscardResult<FileError> CSVParser::open() {
    return m_file.open(OpenFileMode::Read);
}
Result<CSVParser::CSVRH> CSVParser::read_row() {
    bool eof_reached   = false;
    auto line_or_error = m_file.read_line(eof_reached);
    if (line_or_error.is_error()) { return line_or_error.to_error().error_string(); }
    if (m_has_header && !m_read_header) { return CSVHeader::parse_header(line_or_error.to_ok()).map<CSVRH>(); }
    return CSVRow::parse_row(line_or_error.to_ok()).map<CSVRH>();
}
Result<Vector<CSVParser::CSVRH>> CSVParser::read_all() {
    Vector<CSVParser::CSVRH> rows{};
    while (true) {
        if (auto row_or_err = read_row(); row_or_err.is_ok()) {
            rows.append(move(row_or_err.to_ok()));
        } else {
            break;
        }
    }
    return rows;
}
Result<CSVRow> CSVRow::parse_row(String&& line) {
    Vector<String> row;
    // TODO: parse row!
    return CSVRow{ move(row) };
}
Result<CSVHeader> CSVHeader::parse_header(String&& line) {
    Vector<String> header;
    // TODO: parse header!
    return CSVHeader{ move(header) };
}
}    // namespace ARLib