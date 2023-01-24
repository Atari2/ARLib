#pragma once
#include "Compat.h"
#include "TypeTraits.h"
#include "String.h"
#include "WString.h"
#include "StringView.h"
#include "WStringView.h"
#include "arlib_osapi.h"
#include "PrintInfo.h"
namespace ARLib {
using FsString        = ConditionalT<windows_build, WString, String>;
using NonFsString     = ConditionalT<windows_build, String, WString>;
using FsStringView    = ConditionalT<windows_build, WStringView, StringView>;
using NonFsStringView = ConditionalT<windows_build, StringView, WStringView>;
FsString convert_from_non_fs_to_fs(const NonFsString& path);
class Path {
    FsString m_path;
    static auto convert_backslashes_to_native(FsString&& path) {
        constexpr auto native_backslash     = windows_build ? FsChar('\\') : FsChar('/');
        constexpr auto non_native_backslash = windows_build ? FsChar('/') : FsChar('\\');
        for (size_t i = 0; i < path.size(); i++) {
            if (path[i] == non_native_backslash) { path[i] = native_backslash; }
        }
        return path;
    }
    public:
    Path() = default;
    Path(FsString path) : m_path(convert_backslashes_to_native(move(path))) {}
    Path(const NonFsString& path) : m_path(convert_backslashes_to_native(convert_from_non_fs_to_fs(path))) {}
    Path(FsStringView path) : m_path(convert_backslashes_to_native(move(FsString{path}))) {}
    Path(const NonFsStringView& path) : m_path(convert_backslashes_to_native(convert_from_non_fs_to_fs(NonFsString{path}))) {}
    const auto& string() const { return m_path; }
    auto& string() { return m_path; }
    decltype(auto) narrow() const {
#ifdef ON_WINDOWS
        return wstring_to_string(m_path.view());
#else
        return m_path;
#endif
    }
    void remove_filespec();
};
inline Path operator""_p(const wchar_t* path, size_t len) {
    return Path{
        WString{path, len}
    };
}
inline Path operator""_p(const char* path, size_t len) {
    return Path{
        String{path, len}
    };
}
template <>
struct PrintInfo<Path> {
    const Path& m_path;
    PrintInfo(const Path& path) : m_path(path) {}
    String repr() const { return m_path.narrow(); }
};
}    // namespace ARLib