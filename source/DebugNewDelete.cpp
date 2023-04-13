#ifdef DEBUG_NEW_DELETE
    #include "DebugNewDelete.hpp"
    #include "Assertion.hpp"
    #include <cstdlib>
    #include <cstdio>

bool g_stop_collection = false;
bool get_stop_collection() {
    return g_stop_collection;
}
static DebugNewDeleteMap* get_alloc_map() {
    static DebugNewDeleteMap* s_alloc_map = nullptr;
    if (s_alloc_map == nullptr) {
        auto* memptr = std::calloc(1, sizeof(DebugNewDeleteMap));
        s_alloc_map  = new (memptr) DebugNewDeleteMap;
        atexit([]() {
            g_stop_collection = true;
            s_alloc_map->~DebugNewDeleteMap();
            std::free(s_alloc_map);
            s_alloc_map = nullptr;
        });
    }
    return s_alloc_map;
};
    #ifdef ON_WINDOWS
[[nodiscard]] _Ret_notnull_ _Post_writable_byte_size_(count) __declspec(allocator) void* __cdecl
    #else
void*
    #endif
operator new(ARLib::size_t count) {
    if (count == 0) { ++count; }
    if (void* ptr = std::malloc(count)) {
        if (!g_stop_collection && !get_alloc_map()->lost_entries()) {
            get_alloc_map()->insert(ptr, { count, AllocType::Single });
            ::printf("global op new called, size = %zu, ptr for memory at %p (Single)\n", count, ptr);
        }
        return ptr;
    }
    ::printf("global op new failed for size %zu", count);
    abort_arlib();
    unreachable
}
    #ifdef ON_WINDOWS
[[nodiscard]] _Ret_notnull_ _Post_writable_byte_size_(count) __declspec(allocator) void* __cdecl
    #else
void*
    #endif
operator new[](ARLib::size_t count) {
    if (count == 0) { ++count; }
    if (void* ptr = std::malloc(count)) {
        if (!g_stop_collection && !get_alloc_map()->lost_entries()) {
            get_alloc_map()->insert(ptr, { count, AllocType::Multiple });
            ::printf("global op new[] called, size = %zu, ptr for memory at %p (Multiple)\n", count, ptr);
        }
        return ptr;
    }
    ::printf("global op new[] failed for size %zu", count);
    abort_arlib();
    unreachable
}
void operator delete(void* ptr) {
    if (ptr == nullptr) return;
    if (!g_stop_collection && !get_alloc_map()->lost_entries()) {
        auto [size, type] = get_alloc_map()->remove(ptr);
        ::printf(
        "global op delete called on memory at %p with size %zu (%s)\n", ptr, size,
        type == AllocType::Single ? "Single" : "Multiple"
        );
        if (type != AllocType::Single) {
            ::printf(
            "Mismatch deallocation type. Pointer %p was allocated with `new[]` but was deallocated with `delete`\n", ptr
            );
        }
    }
    std::free(ptr);
}
void operator delete[](void* ptr) {
    if (!g_stop_collection && !get_alloc_map()->lost_entries()) {
        if (ptr == nullptr) return;
        auto [size, type] = get_alloc_map()->remove(ptr);
        ::printf(
        "global op delete[] called on memory at %p with size %zu (%s)\n", ptr, size,
        type == AllocType::Single ? "Single" : "Multiple"
        );
        if (type != AllocType::Multiple) {
            ::printf(
            "Mismatch deallocation type. Pointer %p was allocated with `new` but was deallocated with `delete[]`\n", ptr
            );
        }
    }
    std::free(ptr);
}
#endif
