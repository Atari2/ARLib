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
FsString normalize_slashes(FsString&& path);
FsString normalize_slashes(const FsString& path);

template <typename T>
concept PathString = IsAnyOfV<T, FsString, NonFsString>;
class Path {
    FsString m_path;
    static FsString init_path(auto&& path) {
        using T = decltype(path);
        if constexpr (SameAsCvRef<T, FsString>) {
            return normalize_slashes(Forward<T>(path));
        } else if constexpr (SameAsCvRef<T, NonFsString>) {
            return normalize_slashes(convert_from_non_fs_to_fs(path));
        } else {
            static_assert(AlwaysFalse<T>, "Invalid type passed to init_path");
        }
    }
    public:
    Path() = default;
    Path(const FsString& path) : m_path(init_path(path)) {}
    Path(FsString&& path) : m_path(init_path(Forward<FsString>(path))) {}
    Path(const NonFsString& path) : m_path(init_path(path)) {}
    Path(FsStringView path) : m_path(init_path(FsString{ path })) {}
    Path(const NonFsStringView& path) : m_path(init_path(NonFsString{ path })) {}
    bool operator==(const Path& other) const;
    bool operator!=(const Path& other) const { return !(*this == other); }
    Path& operator/=(const Path& other);
    Path operator/(const Path& other) const;
    [[nodiscard]] Path parent_path() const;
    bool is_absolute() const;
    const auto& string() const { return m_path; }
    decltype(auto) narrow() const {
#ifdef ON_WINDOWS
        return wstring_to_string(m_path.view());
#else
        return m_path;
#endif
    }
    [[nodiscard]] Path remove_filespec() const;
    [[nodiscard]] Path extension() const;
    [[nodiscard]] Path filename() const;
    bool is_directory() const;
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