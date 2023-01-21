#pragma once
#include "../../Types.h"
#ifdef UNIX_OR_MINGW
    #include "xnative_fs_unix.h"
#else
    #include "xnative_fs_windows.h"
#endif

#if not defined(FILESYSTEM_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
    #error "Don't include the XNative files directly. Use FileSystem.h"
#endif