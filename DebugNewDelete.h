#pragma once

#ifdef DEBUG_NEW_DELETE
    #include "Types.h"
    #include "Pair.h"
    #include "IntrusiveMap.h"

void* operator new(ARLib::size_t count);
void* operator new[](ARLib::size_t count);
void operator delete(void* ptr);
void operator delete[](void* ptr);
#endif
