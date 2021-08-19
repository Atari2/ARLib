#pragma once

#ifdef DEBUG_NEW_DELETE
#include "Types.h"
#include "Pair.h"
#include "IntrusiveMap.h"


enum class AllocType : int { Single, Multiple };

constexpr ARLib::size_t MAP_SIZE = 400ull;
static ARLib::IntrusiveMap<void*, ARLib::Pair<ARLib::size_t, AllocType>, MAP_SIZE>* s_alloc_map{};
static bool created_once = false;


void* operator new(ARLib::size_t count);
void* operator new[](ARLib::size_t count);
void operator delete(void* ptr);
void operator delete[](void* ptr);
#endif