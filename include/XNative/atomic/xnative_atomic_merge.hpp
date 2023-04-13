#pragma once
#include "Types.hpp"
#ifdef UNIX_OR_MINGW
    #include "XNative/atomic/xnative_atomic_unix.hpp"
#else
    #include "XNative/atomic/xnative_atomic_windows.hpp"
#endif

#if not defined(ATOMIC_INCLUDED__) and not defined(INCLUDED_FROM_OWN_CPP___)
    #error "Don't include the XNative files directly. Use Atomic.h"
#endif
namespace ARLib {}    // namespace ARLib
