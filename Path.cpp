#include "Path.h"
#include "FileSystem.h"
#include "Vector.h"
namespace ARLib {
FsString convert_from_non_fs_to_fs(const NonFsString& path) {
#ifdef ON_WINDOWS
    return string_to_wstring(path.view());
#else
    return wstring_to_string(path.view());
#endif
}
constexpr auto native_backslash     = windows_build ? FsChar('\\') : FsChar('/');
constexpr auto non_native_backslash = windows_build ? FsChar('/') : FsChar('\\');
FsString normalize_slashes(const FsString& path) {
    FsString copy{ path };
    return normalize_slashes(move(copy));
}
FsString normalize_slashes(FsString&& path) {
    bool previous_was_slash = false;
    Vector<FsChar> vec{};
    vec.reserve(path.size());
    for (const auto c : path) {
        if (c != native_backslash && c != non_native_backslash) {
            previous_was_slash = false;
            vec.append(c);
        } else if ((c == native_backslash || c == non_native_backslash) && !previous_was_slash) {
            previous_was_slash = true;
            vec.append(native_backslash);
        }
    }
    return vec.view().collect<FsString>();
}
bool Path::operator==(const Path& other) const {
    return m_path == other.m_path;
}
Path& Path::operator/=(const Path& other) {
    Path copy = *this / other;
    *this     = move(copy);
    return *this;
}
Path Path::operator/(const Path& other) const {
    return Path{ ARLib::combine_paths(m_path, other.m_path) };
}
[[nodiscard]] Path Path::remove_filespec() const {
    FsString copy{ m_path };
    ARLib::remove_filespec(copy);
    return Path{ move(copy) };
};
[[nodiscard]] Path Path::extension() const {
    auto idx = m_path.last_index_of(FsChar{ '.' });
    if (idx == FsString::npos) {
        return Path{};
    } else {
        return Path{ m_path.substring(idx) };
    }
}
[[nodiscard]] Path Path::filename() const {
    auto idx = m_path.last_index_of(native_backslash);
    if (idx == FsString::npos) {
        return Path{ m_path };
    } else {
        return Path{ m_path.substring(idx + 1) };
    }
}
bool Path::is_directory() const {
    return ARLib::is_directory(m_path);
}
[[nodiscard]] Path Path::parent_path() const {
    FsString copy{ m_path };
    ARLib::parent_path(copy);
    return Path{ move(copy) };
}
bool Path::is_absolute() const {
    return ARLib::is_absolute(m_path);
}
}    // namespace ARLib
