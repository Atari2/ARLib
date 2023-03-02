#pragma once

#ifdef DEBUG_NEW_DELETE
    #include "Types.h"
    #include "Pair.h"
    #include "IntrusiveMap.h"
    #ifdef ON_WINDOWS
[[nodiscard]] _Ret_notnull_ _Post_writable_byte_size_(count) __declspec(allocator) void* __cdecl
    #else
void*
    #endif
operator new(ARLib::size_t count);
    #ifdef ON_WINDOWS
[[nodiscard]] _Ret_notnull_ _Post_writable_byte_size_(count) __declspec(allocator) void* __cdecl
    #else
void*
    #endif
operator new[](ARLib::size_t count);
void operator delete(void* ptr);
void operator delete[](void* ptr);
#endif
