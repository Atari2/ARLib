#include "CSVParser.hpp"
#include "Printer.hpp"
namespace ARLib {
CSVResult CSVParser::read_row() {
    auto res = CSVRow::parse_row(m_stream.get(), move(m_leftover), m_separator, *this);
    if (res.is_error()) { return res.to_error(); }
    auto&& [row, rest] = res.to_ok();
    m_leftover         = move(rest);
    m_leftover.iltrim();
    if (m_has_header && !m_header->has_value()) {
        m_header->emplace(move(row.m_row));
        return read_row();
    }
    return row;
}
Result<Vector<CSVResult>, CSVParseError> CSVParser::read_all() {
    Vector<CSVResult> rows{};
    while (true) {
        if (auto row_or_err = read_row(); row_or_err.is_ok()) {
            auto res = row_or_err.to_ok();
            rows.append(move(res));
        } else {
            auto err = row_or_err.to_error();
            if (err->is_eof()) break;
            return err;
        }
    }
    return rows;
}
static Result<Tuple<String, String, bool>, CSVParseError> parse_field(CharacterStream* file, String&& leftover, char sep) {
    bool eof_reached = false;
    auto res         = file->read_line(eof_reached);
    if (res.is_error()) {
        auto err = res.to_error();
        return CSVParseError{ err->error_string(), file->pos() };
    }
    auto line = res.to_ok();
    // read_line eats the CRLF, so we add it back
    line = leftover + line + "\r\n"_s;
    if (line.size() == 2 && eof_reached) {
        // both leftover and line are empty, "\r\n" is the only thing
        // let's also check eof_reached
        return Tuple{ ""_s, ""_s, true };
    }
    auto it           = line.begin();
    auto end          = line.end();
    size_t parsed_len = 0;
    if (it != end && *it == sep) {
        ++it;
        parsed_len++;
    }
    bool in_double_quotes = false;
    String field{};
    while (it != end) {
        char cur = *it;
        if (cur == '"') {
            if (in_double_quotes) {
                // quote inside a double quoted string
                ++it;
                if (it == end) {
                    return CSVParseError{ "Unexpected end of field"_s, file->pos() };
                } else {
                    char c = *it;
                    if (c == '"') {
                        // escaped
                        field.append('"');
                        parsed_len += 2;
                    } else {
                        // end of field

                        parsed_len += 2;
                        // scan for next ',' (or separator), skipping spaces and tabs
                        while (it != end && (*it == ' ' || *it == '\t')) {
                            ++it;
                            ++parsed_len;
                        }
                        if (*it == sep) {
                            return Tuple{ move(field), line.substring(parsed_len), false };
                        } else {
                            return Tuple{ move(field), line.substring(parsed_len), true };
                        }
                    }
                }
            } else if (!field.is_empty()) {
                return CSVParseError{ "Stray double quote inside of field not enclosed in double quotes"_s,
                                      file->pos() };
            }
            // field is enclosed in double quotes
            in_double_quotes = true;
            ++it;
        } else if (cur == sep) {
            if (in_double_quotes) {
                field.append(sep);
                parsed_len++;
                ++it;
            } else {
                // end of field
                return Tuple{ move(field), line.substring(parsed_len), false };
            }
        } else if (cur == '\r') {
            if (in_double_quotes) {
                field.append('\r');
                parsed_len++;
            }
            ++it;
        } else if (cur == '\n') {
            if (in_double_quotes) {
                field.append('\n');
                parsed_len++;
            } else {
                parsed_len++;
                return Tuple{ move(field), line.substring(parsed_len), true };
            }
            ++it;
        } else {
            field.append(cur);
            parsed_len++;
            ++it;
        }
    }
    return CSVParseError{ "Unexpected end of line"_s, file->pos() };
}
CSVRowResult CSVRow::parse_row(CharacterStream* file, String&& from_last, char sep, const CSVParser& parser) {
    Vector<String> row;
    String leftover{ move(from_last) };
    while (true) {
        auto r = parse_field(file, move(leftover), sep);
        if (r.is_ok()) {
            auto&& [field, left, end_of_row] = r.to_ok();
            leftover                         = move(left);
            if (field.is_empty() && end_of_row) break;
            row.append(move(field));
            if (end_of_row) break;
        } else {
            return r.to_error();
        }
    }
    if (row.size() == 0) { return CSVEndOfFileError{}; }
    return Pair{
        CSVRow{move(row), parser.header().weakptr()},
        leftover
    };
}
}    // namespace ARLib