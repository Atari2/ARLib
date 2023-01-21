#pragma once
#include "../../Compat.h"
#ifdef WINDOWS
    #if not defined(FILESYSTEM_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
        #error "Don't include the XNative files directly. Use FileSystem.h"
    #endif
namespace ARLib {
}    // namespace ARLib
#endif
