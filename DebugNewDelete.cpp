#ifdef DEBUG_NEW_DELETE
#include "DebugNewDelete.h"
#include "Assertion.h"
#include "cstdio_compat.h"
#include <cstdlib>

using MapType = ARLib::IntrusiveMap<void*, ARLib::Pair<ARLib::size_t, AllocType>, MAP_SIZE>;

void check_and_create_map() {
    if (!s_alloc_map && !created_once) {
        s_alloc_map = reinterpret_cast<MapType*>(std::calloc(1, sizeof(MapType)));
        auto lam = []() {
            s_alloc_map->~IntrusiveMap();
            std::free(s_alloc_map);
            s_alloc_map = nullptr;
        };
        atexit(lam);
        created_once = true;
    }
}

void* operator new(ARLib::size_t count) {
    if (count == 0) { ++count; }
    if (void* ptr = std::malloc(count)) {
        check_and_create_map();
        s_alloc_map->insert(ptr, {count, AllocType::Single});
        ARLib::printf("global op new called, size = %zu, ptr for memory at %p (Single)\n", count, ptr);
        return ptr;
    }
    ARLib::printf("global op new failed for size %zu", count);
    abort_arlib();
    unreachable
}
void* operator new[](ARLib::size_t count) {
    if (count == 0) { ++count; }
    if (void* ptr = std::malloc(count)) {
        check_and_create_map();
        s_alloc_map->insert(ptr, {count, AllocType::Multiple});
        ARLib::printf("global op new[] called, size = %zu, ptr for memory at %p (Multiple)\n", count, ptr);
        return ptr;
    }
    ARLib::printf("global op new[] failed for size %zu", count);
    abort_arlib();
    unreachable
}
void operator delete(void* ptr) {
    if (ptr == nullptr) return;
    check_and_create_map();
    auto [size, type] = s_alloc_map->remove(ptr);
    ARLib::printf("global op delete called on memory at %p with size %zu (%s)\n", ptr, size,
                  type == AllocType::Single ? "Single" : "Multiple");
    if (type != AllocType::Single) {
        ARLib::printf(
        "Mismatch deallocation type. Pointer was allocated with `new[]` but was deallocated with `delete`\n");
    }
    std::free(ptr);
}
void operator delete[](void* ptr) {
    if (ptr == nullptr) return;
    check_and_create_map();
    auto [size, type] = s_alloc_map->remove(ptr);
    ARLib::printf("global op delete[] called on memory at %p with size %zu (%s)\n", ptr, size,
                  type == AllocType::Single ? "Single" : "Multiple");
    if (type != AllocType::Multiple) {
        ARLib::printf(
        "Mismatch deallocation type. Pointer was allocated with `new` but was deallocated with `delete[]`\n");
    }
    std::free(ptr);
}
#endif