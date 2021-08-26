#ifdef DEBUG_NEW_DELETE
#include "DebugNewDelete.h"
#include "Assertion.h"
#include "cstdio_compat.h"
#include <cstdlib>

static DebugNewDeleteMap* get_alloc_map() {
    static bool s_created = false;
    static DebugNewDeleteMap* s_alloc_map = static_cast<DebugNewDeleteMap*>(std::calloc(1, sizeof(DebugNewDeleteMap)));
    if (!s_created) {
        atexit([]() {
            s_alloc_map->~IntrusiveMap<void*, ARLib::Pair<size_t, AllocType>, MAP_SIZE>();
            std::free(s_alloc_map);
            s_alloc_map = nullptr;
        });
        s_created = true;
    }
    return s_alloc_map;
};

void* operator new(ARLib::size_t count) {
    if (count == 0) { ++count; }
    if (void* ptr = std::malloc(count)) {
        get_alloc_map()->insert(ptr, {count, AllocType::Single});
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
        get_alloc_map()->insert(ptr, {count, AllocType::Multiple});
        ARLib::printf("global op new[] called, size = %zu, ptr for memory at %p (Multiple)\n", count, ptr);
        return ptr;
    }
    ARLib::printf("global op new[] failed for size %zu", count);
    abort_arlib();
    unreachable
}
void operator delete(void* ptr) {
    if (ptr == nullptr) return;
    auto [size, type] = get_alloc_map()->remove(ptr);
    ARLib::printf("global op delete called on memory at %p with size %zu (%s)\n", ptr, size,
                  type == AllocType::Single ? "Single" : "Multiple");
    if (type != AllocType::Single) {
        ARLib::printf(
        "Mismatch deallocation type. Pointer %p was allocated with `new[]` but was deallocated with `delete`\n", ptr);
    }
    std::free(ptr);
}
void operator delete[](void* ptr) {
    if (ptr == nullptr) return;
    auto [size, type] = get_alloc_map()->remove(ptr);
    ARLib::printf("global op delete[] called on memory at %p with size %zu (%s)\n", ptr, size,
                  type == AllocType::Single ? "Single" : "Multiple");
    if (type != AllocType::Multiple) {
        ARLib::printf(
        "Mismatch deallocation type. Pointer %p was allocated with `new` but was deallocated with `delete[]`\n", ptr);
    }
    std::free(ptr);
}
#endif