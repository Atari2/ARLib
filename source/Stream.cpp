#include "Stream.hpp"
#include "Vector.hpp"
namespace ARLib {
// CHARACTER STREAM ITERATORS
CharacterStream::LinesIterator CharacterStream::Lines::begin() {
    return CharacterStream::LinesIterator{ m_stream };
}
CharacterStream::LinesIterator CharacterStream::Lines::end() {
    return CharacterStream::LinesIterator{ m_stream, true };
}
CharacterStream::LinesIterator CharacterStream::Lines::begin() const {
    return CharacterStream::LinesIterator{ m_stream };
}
CharacterStream::LinesIterator CharacterStream::Lines::end() const {
    return CharacterStream::LinesIterator{ m_stream, true };
}
CharacterStream::LinesIterator::LinesIterator(MaybeOwned<CharacterStream> stream, bool end) :
    m_stream{ move(stream) }, m_end{ end } {
    if (!m_end) {
        auto readres = m_stream->read_line(m_eof_reached);
        if (readres.is_error()) {
            m_end = true;
            readres.ignore_error();
        } else {
            m_current_line = readres.to_ok();
        }
    }
}
bool CharacterStream::LinesIterator::operator==(const CharacterStream::LinesIterator& other) const {
    return m_end == other.m_end && m_stream == other.m_stream;
}
bool CharacterStream::LinesIterator::operator!=(const CharacterStream::LinesIterator& other) const {
    return m_end != other.m_end || m_stream != other.m_stream;
}
String CharacterStream::LinesIterator::operator*() {
    return m_current_line;
}
CharacterStream::LinesIterator& CharacterStream::LinesIterator::operator++() {
    if (m_eof_reached) { m_end = true; }
    if (!m_end) {
        auto readres = m_stream->read_line(m_eof_reached);
        if (readres.is_error()) {
            m_end = true;
        } else {
            m_current_line = readres.to_ok();
        }
    }
    return *this;
}
CharacterStream::LinesIterator CharacterStream::LinesIterator::operator++(int) {
    auto copy = *this;
    ++(*this);
    return copy;
}

#define RETURN_ERROR_STRING(res)                                                                                       \
    if (res.is_error()) {                                                                                              \
        auto error = res.to_error();                                                                                   \
        return error->error_string();                                                                                  \
    }
// FILE STREAM
DiscardResult<FileError> FileStream::open() {
    return m_file.open(OpenFileMode::Append | OpenFileMode::Read);
}
Result<size_t> FileStream::write(Span<const uint8_t> buffer) {
    StringView view{ reinterpret_cast<const char*>(buffer.data()), buffer.size_bytes() };
    auto res = m_file.write(view);
    RETURN_ERROR_STRING(res);
    return res.to_ok();
}
Result<size_t> FileStream::write(Span<const char> buffer) {
    StringView view{ buffer.data(), buffer.size_bytes() };
    auto res = m_file.write(view);
    RETURN_ERROR_STRING(res);
    return res.to_ok();
}
Result<size_t> FileStream::write(StringView view) {
    auto res = m_file.write(view);
    RETURN_ERROR_STRING(res);
    return res.to_ok();
}
Result<Vector<uint8_t>> FileStream::read(size_t n) {
    auto res = m_file.read_n(n);
    RETURN_ERROR_STRING(res);
    auto ok         = res.to_ok();
    const size_t sz = ok.size();
    uint8_t* ptr    = reinterpret_cast<uint8_t*>(ok.release());
    Vector<uint8_t> vec{ ptr, sz };
    return vec;
}
Result<Vector<uint8_t>> FileStream::read() {
    auto res = m_file.read_all();
    RETURN_ERROR_STRING(res);
    auto ok         = res.to_ok();
    const size_t sz = ok.size();
    uint8_t* ptr    = reinterpret_cast<uint8_t*>(ok.release());
    Vector<uint8_t> vec{ ptr, sz };
    return vec;
}
Result<size_t> FileStream::write_string(StringView buffer) {
    auto res = m_file.write(buffer);
    RETURN_ERROR_STRING(res);
    return res.to_ok();
}
Result<String> FileStream::read_string() {
    auto res = m_file.read_all();
    if (res.is_error()) {
        auto error = res.to_error();
        return Result<String>{ error->error_string(), emplace_error };
    }
    return Result<String>{ res.to_ok(), emplace_ok };
}
Result<String> FileStream::read_line(bool& eof_reached) {
    auto res = m_file.read_line(eof_reached);
    if (res.is_error()) {
        auto error = res.to_error();
        return Result<String>{ error->error_string(), emplace_error };
    }
    return Result<String>{ res.to_ok(), emplace_ok };
}
size_t FileStream::pos() const {
    return m_file.pos();
}
size_t FileStream::seek(size_t pos) {
    return m_file.seek(pos);
}
bool FileStream::operator==(const FileStream& other) const {
    return m_file == other.m_file;
}
// BUFFERED FILE STREAM
Result<size_t> BufferedFileStream::write(Span<const uint8_t> buffer) {
    StringView view{ reinterpret_cast<const char*>(buffer.data()), buffer.size_bytes() };
    size_t rem_buffer = m_buffer_capacity - m_buffer.size();
    if (buffer.size() <= rem_buffer) {
        // we can still fit the entire to-write buffer into our inline buffer
        m_buffer.append(view);
    } else {
        // append what we can fit, flush the buffer to file, and then write the rest
        m_buffer.append(view.substring(rem_buffer));
        auto res = m_file.write(m_buffer);
        m_buffer.clear();
        m_buffer.append(view.substringview_fromlen(rem_buffer));
        if (res.is_error()) {
            auto error = res.to_error();
            return error->error_string();
        }
    }
    return view.size();
}
Result<size_t> BufferedFileStream::write(Span<const char> buffer) {
    StringView view{ buffer.data(), buffer.size_bytes() };
    size_t rem_buffer = m_buffer_capacity - m_buffer.size();
    if (buffer.size() <= rem_buffer) {
        // we can still fit the entire to-write buffer into our inline buffer
        m_buffer.append(view);
    } else {
        // append what we can fit, flush the buffer to file, and then write the rest
        m_buffer.append(view.substring(rem_buffer));
        auto res = m_file.write(m_buffer);
        m_buffer.clear();
        m_buffer.append(view.substringview_fromlen(rem_buffer));
        if (res.is_error()) {
            auto error = res.to_error();
            return error->error_string();
        }
    }
    return view.size();
}
Result<size_t> BufferedFileStream::write(StringView view) {
    size_t rem_buffer = m_buffer_capacity - m_buffer.size();
    if (view.size() <= rem_buffer) {
        // we can still fit the entire to-write buffer into our inline buffer
        m_buffer.append(view);
    } else {
        // append what we can fit, flush the buffer to file, and then write the rest
        m_buffer.append(view.substring(rem_buffer));
        auto res = m_file.write(m_buffer);
        m_buffer.clear();
        m_buffer.append(view.substringview_fromlen(rem_buffer));
        if (res.is_error()) {
            auto error = res.to_error();
            return error->error_string();
        }
    }
    return view.size();
}
Result<Vector<uint8_t>> BufferedFileStream::read(size_t n) {
    if (m_buffer.size() >= n) {
        // we have enough chars in the buffer, read from that
        auto res = m_buffer.substring(0, n);
        m_buffer.set_size(m_buffer.size() - n);
        const size_t sz = res.size();
        uint8_t* ptr    = reinterpret_cast<uint8_t*>(res.release());
        Vector<uint8_t> vec{ ptr, sz };
        return vec;
    } else {
        size_t remaining = n - m_buffer.size();
        auto first       = m_buffer.substring(0, n);
        TRY_SET(rest, m_file.read_n(remaining));
        rest += first;
        const size_t sz = rest.size();
        uint8_t* ptr    = reinterpret_cast<uint8_t*>(rest.release());
        Vector<uint8_t> vec{ ptr, sz };
        return vec;
    }
}
Result<Vector<uint8_t>> BufferedFileStream::read() {
    TRY_SET(file_data, m_file.read_all());
    file_data += m_buffer;
    m_buffer.set_size(0);
    const size_t sz = file_data.size();
    uint8_t* ptr    = reinterpret_cast<uint8_t*>(file_data.release());
    Vector<uint8_t> vec{ ptr, sz };
    return vec;
}
Result<size_t> BufferedFileStream::write_string(StringView buffer) {
    return write(buffer.bytespan());
}
Result<String> BufferedFileStream::read_string() {
    TRY_SET(file_data, m_file.read_all());
    file_data += m_buffer;
    m_buffer.set_size(0);
    return Result<String>{ file_data, emplace_ok };
}
Result<String> BufferedFileStream::read_line(bool& eof_reached) {
    if (auto index = m_buffer.index_of('\n'); index != String::npos) {
        auto line = m_buffer.substring(0, index);
        m_buffer.set_size(index);
        return Result<String>{ line, emplace_ok };
    } else {
        TRY_SET(line, m_file.read_line(eof_reached));
        line += m_buffer;
        m_buffer.set_size(0);
        return Result<String>{ line, emplace_ok };
    }
}
size_t BufferedFileStream::pos() const {
    return m_file.pos() + m_buffer.size();
}
size_t BufferedFileStream::seek(size_t pos) {
    // seek makes no sense if we don't flush first
    flush();
    return m_file.seek(pos);
}
void BufferedFileStream::flush() {
    if (m_buffer.size() > 0) {
        m_file.write(m_buffer);
        m_buffer.set_size(0);
    }
}
BufferedFileStream::~BufferedFileStream() {
    flush();
}
// STRING VIEW STREAM
StringViewStream::LinesViewIterator StringViewStream::LinesView::begin() {
    return StringViewStream::LinesViewIterator{ m_stream };
}
StringViewStream::LinesViewIterator StringViewStream::LinesView::end() {
    return StringViewStream::LinesViewIterator{ m_stream, true };
}
StringViewStream::LinesViewIterator StringViewStream::LinesView::begin() const {
    return StringViewStream::LinesViewIterator{ m_stream };
}
StringViewStream::LinesViewIterator StringViewStream::LinesView::end() const {
    return StringViewStream::LinesViewIterator{ m_stream, true };
}
StringViewStream::LinesViewIterator::LinesViewIterator(MaybeOwned<StringViewStream> stream, bool end) :
    m_stream{ move(stream) }, m_end{ end } {
    if (!m_end) {
        auto readres = m_stream->read_line_view(m_eof_reached);
        if (readres.is_error()) {
            m_end = true;
            readres.ignore_error();
        } else {
            m_current_line = readres.to_ok();
        }
    }
}
bool StringViewStream::LinesViewIterator::operator==(const StringViewStream::LinesViewIterator& other) const {
    return m_end == other.m_end && m_stream == other.m_stream;
}
bool StringViewStream::LinesViewIterator::operator!=(const StringViewStream::LinesViewIterator& other) const {
    return m_end != other.m_end || m_stream != other.m_stream;
}
StringView StringViewStream::LinesViewIterator::operator*() {
    return m_current_line;
}
StringViewStream::LinesViewIterator& StringViewStream::LinesViewIterator::operator++() {
    if (m_eof_reached) { m_end = true; }
    if (!m_end) {
        auto readres = m_stream->read_line_view(m_eof_reached);
        if (readres.is_error()) {
            m_end = true;
        } else {
            m_current_line = readres.to_ok();
        }
    }
    return *this;
}
StringViewStream::LinesViewIterator StringViewStream::LinesViewIterator::operator++(int) {
    auto copy = *this;
    ++(*this);
    return copy;
}
Span<char> StringViewStream::_writeable_span() {
    // this const_cast is fine because the pointer underlying the StringView is writable when created
    // it's just necessary because the StringView itself doesn't allow modifications.
    return Span<char>{ const_cast<char*>(m_view.data()), m_view.size() };
}
Result<size_t> StringViewStream::write(Span<const uint8_t> buffer) {
    StringView bufview{ reinterpret_cast<const char*>(buffer.data()), buffer.size_bytes() };
    auto writeable_span = _writeable_span();
    int64_t to_write   = static_cast<int64_t>(bufview.size());
    int64_t space_left = static_cast<int64_t>(m_view.size() - m_pos);
    if (auto needed_space = to_write - space_left; needed_space > 0) {
        return Error{ "Cannot write to StringViewStream: not enough space" };
    }
    for (size_t i = 0; to_write > 0; --to_write, ++m_pos, ++i) { writeable_span[m_pos] = bufview[i]; }
    return static_cast<size_t>(to_write);
}
Result<size_t> StringViewStream::write(Span<const char> buffer) {
    StringView bufview{ buffer.data(), buffer.size_bytes() };
    auto writeable_span = _writeable_span();
    int64_t to_write   = static_cast<int64_t>(bufview.size());
    int64_t space_left = static_cast<int64_t>(m_view.size() - m_pos);
    if (auto needed_space = to_write - space_left; needed_space > 0) {
        return Error{ "Cannot write to StringViewStream: not enough space" };
    }
    for (size_t i = 0; to_write > 0; --to_write, ++m_pos, ++i) { writeable_span[m_pos] = bufview[i]; }
    return static_cast<size_t>(to_write);
}
Result<size_t> StringViewStream::write(StringView bufview) {
    int64_t to_write    = static_cast<int64_t>(bufview.size());
    auto writeable_span = _writeable_span();
    int64_t space_left = static_cast<int64_t>(m_view.size() - m_pos);
    if (auto needed_space = to_write - space_left; needed_space > 0) {
        return Error{ "Cannot write to StringViewStream: not enough space" };
    }
    for (size_t i = 0; to_write > 0; --to_write, ++m_pos, ++i) { writeable_span[m_pos] = bufview[i]; }
    return static_cast<size_t>(to_write);
}
Result<Vector<uint8_t>> StringViewStream::read(size_t n) {
    size_t left = m_view.size() - m_pos;
    if (n > left) { n = left; }
    String repr = m_view.substringview(m_pos, m_pos + n).str();
    auto* ptr   = reinterpret_cast<uint8_t*>(repr.release());
    return Vector<uint8_t>{ ptr, m_view.size() };
}
Result<Vector<uint8_t>> StringViewStream::read() {
    String repr = m_view.str();
    auto* ptr   = reinterpret_cast<uint8_t*>(repr.release());
    return Vector<uint8_t>{ ptr, m_view.size() };
}
Result<size_t> StringViewStream::write_string(StringView buffer) {
    auto writeable_span = _writeable_span();
    int64_t to_write   = static_cast<int64_t>(buffer.size());
    int64_t space_left = static_cast<int64_t>(m_view.size() - m_pos);
    if (auto needed_space = to_write - space_left; needed_space > 0) {
        return Error{ "Cannot write to StringViewStream: not enough space" };
    }
    for (size_t i = 0; to_write > 0; --to_write, ++m_pos, ++i) { writeable_span[m_pos] = buffer[i]; }
    return static_cast<size_t>(to_write);
}
Result<String> StringViewStream::read_string() {
    return Result<String>{ str(), emplace_ok };
}
Result<String> StringViewStream::read_line(bool& eof_reached) {
    size_t pos_of_n = m_view.index_of('\n', m_pos);
    if (pos_of_n == String::npos) {
        eof_reached = true;
        auto line   = m_view.substring(m_pos);
        m_pos       = m_view.size();
        return { move(line), emplace_ok };
    }
    auto ret = m_view.substringview(m_pos, pos_of_n);
    m_pos    = pos_of_n + 1;
    return { ret, emplace_ok };
}
Result<StringView> StringViewStream::read_string_view() {
    return Result<StringView>{ m_view, emplace_ok };
}
Result<StringView> StringViewStream::read_line_view(bool& eof_reached) {
    size_t pos_of_n = m_view.index_of('\n', m_pos);
    if (pos_of_n == StringView::npos) {
        eof_reached = true;
        auto line   = m_view.substring(m_pos);
        m_pos       = m_view.size();
        return { move(line), emplace_ok };
    }
    auto ret = m_view.substringview(m_pos, pos_of_n);
    m_pos    = pos_of_n + 1;
    return { ret, emplace_ok };
}
// STRING STREAM
Result<size_t> StringStream::write(Span<const uint8_t> buffer) {
    StringView bufview{ reinterpret_cast<const char*>(buffer.data()), buffer.size_bytes() };
    int64_t to_write   = static_cast<int64_t>(bufview.size());
    int64_t space_left = static_cast<int64_t>(m_buffer.size() - m_pos);
    if (auto needed_space = to_write - space_left; needed_space > 0) {
        m_buffer.resize(m_buffer.size() + static_cast<size_t>(needed_space));
    }
    for (size_t i = 0; to_write > 0; --to_write, ++m_pos, ++i) { m_buffer[m_pos] = bufview[i]; }
    return static_cast<size_t>(to_write);
}
Result<size_t> StringStream::write(Span<const char> buffer) {
    StringView bufview{ buffer.data(), buffer.size_bytes() };
    int64_t to_write   = static_cast<int64_t>(bufview.size());
    int64_t space_left = static_cast<int64_t>(m_buffer.size() - m_pos);
    if (auto needed_space = to_write - space_left; needed_space > 0) {
        m_buffer.resize(m_buffer.size() + static_cast<size_t>(needed_space));
    }
    for (size_t i = 0; to_write > 0; --to_write, ++m_pos, ++i) { m_buffer[m_pos] = bufview[i]; }
    return static_cast<size_t>(to_write);
}
Result<size_t> StringStream::write(StringView bufview) {
    int64_t to_write   = static_cast<int64_t>(bufview.size());
    int64_t space_left = static_cast<int64_t>(m_buffer.size() - m_pos);
    if (auto needed_space = to_write - space_left; needed_space > 0) {
        m_buffer.resize(m_buffer.size() + static_cast<size_t>(needed_space));
    }
    for (size_t i = 0; to_write > 0; --to_write, ++m_pos, ++i) { m_buffer[m_pos] = bufview[i]; }
    return static_cast<size_t>(to_write);
}
Result<Vector<uint8_t>> StringStream::read(size_t n) {
    size_t left = m_buffer.size() - m_pos;
    if (n > left) { n = left; }
    String repr = m_buffer.substring(m_pos, m_pos + n);
    auto* ptr   = reinterpret_cast<uint8_t*>(repr.release());
    return Vector<uint8_t>{ ptr, m_buffer.size() };
}
Result<Vector<uint8_t>> StringStream::read() {
    String repr = m_buffer;
    auto* ptr   = reinterpret_cast<uint8_t*>(repr.release());
    return Vector<uint8_t>{ ptr, m_buffer.size() };
}
Result<size_t> StringStream::write_string(StringView buffer) {
    int64_t to_write   = static_cast<int64_t>(buffer.size());
    int64_t space_left = static_cast<int64_t>(m_buffer.size() - m_pos);
    if (auto needed_space = to_write - space_left; needed_space > 0) {
        m_buffer.resize(m_buffer.size() + static_cast<size_t>(needed_space));
    }
    for (size_t i = 0; to_write > 0; --to_write, ++m_pos, ++i) { m_buffer[m_pos] = buffer[i]; }
    return static_cast<size_t>(to_write);
}
Result<String> StringStream::read_string() {
    return Result<String>{ str(), emplace_ok };
}
Result<String> StringStream::read_line(bool& eof_reached) {
    size_t pos_of_n = m_buffer.index_of('\n', m_pos);
    if (pos_of_n == String::npos) {
        eof_reached = true;
        auto line   = m_buffer.substring(m_pos);
        m_pos       = m_buffer.size();
        return { move(line), emplace_ok };
    }
    auto ret = m_buffer.substring(m_pos, pos_of_n);
    m_pos    = pos_of_n + 1;
    return { ret, emplace_ok };
}
}    // namespace ARLib