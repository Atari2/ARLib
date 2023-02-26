#pragma once
#include "../../Types.h"
#ifdef UNIX_OR_MINGW
    #include "xnative_atomic_unix.h"
#else
    #include "xnative_atomic_windows.h"
#endif

#if not defined(ATOMIC_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
    #error "Don't include the XNative files directly. Use Atomic.h"
#endif
namespace ARLib {}    // namespace ARLib
