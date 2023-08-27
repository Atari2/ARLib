#include "CSVParser.hpp"
namespace ARLib {
DiscardResult<FileError> CSVParser::open() {
    return m_file.open(OpenFileMode::Read);
}
CSVResult CSVParser::read_row() {
    if (m_has_header && !m_read_header) { return CSVHeader::parse_header(m_file).map<CSVRH>(); }
    return CSVRow::parse_row(m_file).map<CSVRH>();
}
Result<Vector<CSVRH>, CSVParseError> CSVParser::read_all() {
    Vector<CSVRH> rows{};
    while (true) {
        if (auto row_or_err = read_row(); row_or_err.is_ok()) {
            rows.append(move(row_or_err.to_ok()));
        } else {
            return row_or_err.to_error();
        }
    }
    return rows;
}
CSVRowResult CSVRow::parse_row([[maybe_unused]] File& file) {
    Vector<String> row;
    // TODO: parse row!
    return CSVRow{ move(row) };
}
CSVHeaderResult CSVHeader::parse_header([[maybe_unused]] File& file) {
    Vector<String> header;
    // TODO: parse header!
    return CSVHeader{ move(header) };
}
}    // namespace ARLib