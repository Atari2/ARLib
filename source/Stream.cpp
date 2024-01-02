#include "Stream.hpp"
#include "Vector.hpp"
namespace ARLib {
DiscardResult<FileError> FileStream::open() {
    return m_file.open(OpenFileMode::Append);
}
Result<size_t> FileStream::write(Span<const uint8_t> buffer) {
    StringView view{ reinterpret_cast<const char*>(buffer.data()), buffer.size_bytes() };
    auto res = m_file.write(view);
    if (res.is_error()) return res.to_error()->error_string();
    return res.to_ok();
}
Result<Vector<uint8_t>> FileStream::read(size_t n) {
    auto res = m_file.read_n(n);
    if (res.is_error()) return res.to_error()->error_string();
    auto ok         = res.to_ok();
    const size_t sz = ok.size();
    uint8_t* ptr    = reinterpret_cast<uint8_t*>(ok.release());
    Vector<uint8_t> vec{ ptr, sz };
    return vec;
}
Result<Vector<uint8_t>> FileStream::read() {
    auto res = m_file.read_all();
    if (res.is_error()) return res.to_error()->error_string();
    auto ok         = res.to_ok();
    const size_t sz = ok.size();
    uint8_t* ptr    = reinterpret_cast<uint8_t*>(ok.release());
    Vector<uint8_t> vec{ ptr, sz };
    return vec;
}
Result<size_t> FileStream::write_string(StringView buffer) {
    auto res = m_file.write(buffer);
    if (res.is_error()) return res.to_error()->error_string();
    return res.to_ok();
}
Result<String> FileStream::read_string() {
    auto res = m_file.read_all();
    if (res.is_error()) return Result<String>{ res.to_error()->error_string(), emplace_error };
    return Result<String>{ res.to_ok(), emplace_ok };
}
Result<String> FileStream::read_line(bool& eof_reached) {
    auto res = m_file.read_line(eof_reached);
    if (res.is_error()) return Result<String>{ res.to_error()->error_string(), emplace_error };
    return Result<String>{ res.to_ok(), emplace_ok };
}
size_t FileStream::pos() const {
    return m_file.pos();
}
size_t FileStream::seek(size_t pos) {
    return m_file.seek(pos);
}
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
    m_pos = pos_of_n + 1;
    return { ret, emplace_ok };
}
}    // namespace ARLib