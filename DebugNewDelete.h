#pragma once

#ifdef DEBUG_NEW_DELETE
#include "Types.h"
#include "IntrusiveMap.h"

constexpr ARLib::size_t MAP_SIZE = 400ull;
static ARLib::IntrusiveMap<void*, ARLib::size_t, MAP_SIZE> s_alloc_map{};

void* operator new(ARLib::size_t count);
void* operator new[](ARLib::size_t count);
void operator delete(void* ptr);
void operator delete[](void* ptr);
#endif