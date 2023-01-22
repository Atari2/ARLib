#include "Path.h"
#include "FileSystem.h"
namespace ARLib {
FsString convert_from_non_fs_to_fs(const NonFsString& path) {
#ifdef ON_WINDOWS
    return string_to_wstring(path.view());
#else
    return wstring_to_string(path.view());
#endif
}
void Path::remove_filespec() {
    ARLib::remove_filespec(m_path);
};
}    // namespace ARLib
