#pragma once
#include "Types.hpp"
#ifdef UNIX_OR_MINGW
    #include "xnative_fs_unix.hpp"
#else
    #include "xnative_fs_windows.hpp"
#endif

#if not defined(FILESYSTEM_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
    #error "Don't include the XNative files directly. Use FileSystem.h"
#endif
namespace ARLib {
#ifdef UNIX_OR_MINGW
using NativeDirectoryIterator = UnixDirectoryIterator;
using NativeDirectoryIterate  = UnixDirectoryIterate;
using NativeFileInfo          = UnixFileInfo;
#else
using NativeDirectoryIterator = Win32DirectoryIterator;
using NativeDirectoryIterate  = Win32DirectoryIterate;
using NativeFileInfo          = Win32FileInfo;
#endif
}    // namespace ARLib