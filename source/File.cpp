#include "File.hpp"
#include <cstdio>
namespace ARLib {
static size_t found_eol(String& line) {
    if (auto eolidx = line.index_of('\n'); eolidx != String::npos) {
        line = line.substring(0, eolidx - (eolidx > 0 ? line[eolidx - 1] == '\r' : 0));
        return eolidx + 1;
    }
    return String::npos;
}
File::ReadResult File::read_line(bool& eof_reached) {
    if (m_mode != OpenFileMode::Read) {
        return FileError{ "Can't read from a file not open in read mode"_s, m_filename };
    }
    String line{};
    String partial{ LINELENGTH_MAX, '\0' };
    do {
        size_t read = ARLib::fread(partial.rawptr(), sizeof(char), LINELENGTH_MAX, m_ptr);
        partial.set_size(read);
        if (size_t aftereolidx = found_eol(partial); aftereolidx != String::npos) {
            // found eol, seek file pointer to just after it
            int64_t offset = -(static_cast<int64_t>(read) - static_cast<int64_t>(aftereolidx));
            auto old_pos   = ARLib::ftell(m_ptr);
            auto new_pos   = ARLib::fseek(m_ptr, static_cast<long>(offset), SEEK_CUR);
            HARD_ASSERT(old_pos - new_pos == -offset, "pebnis");
            line += move(partial);
            break;
        }
        if (read < LINELENGTH_MAX) {
            // read size was less than LINELENGTH_MAX and eol was not found => eof reached
            line += move(partial);
            eof_reached = true;
            break;
        }
        line += move(partial);
    } while (true);
    return line;
}
}    // namespace ARLib
