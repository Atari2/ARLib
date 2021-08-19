#ifdef DEBUG_NEW_DELETE
#include "DebugNewDelete.h"
#include "Assertion.h"
#include "cstdio_compat.h"
#include <cstdlib>

void* operator new(ARLib::size_t count) {
    if (count == 0) { ++count; }
    if (void* ptr = std::malloc(count)) {
        s_alloc_map.insert(ptr, count);
        ARLib::printf("global op new called, size = %zu, ptr for memory at %p\n", count, ptr);
        return ptr;
    }
    ARLib::printf("global op new failed for size %zu", count);
    abort_arlib();
    unreachable
}
void* operator new[](ARLib::size_t count) {
    if (count == 0) { ++count; }
    if (void* ptr = std::malloc(count)) {
        s_alloc_map.insert(ptr, count);
        ARLib::printf("global op new[] called, size = %zu, ptr for memory at %p\n", count, ptr);
        return ptr;
    }
    ARLib::printf("global op new[] failed for size %zu", count);
    abort_arlib();
    unreachable
}
void operator delete(void* ptr) {
    if (ptr == nullptr) return;
    auto size = s_alloc_map.remove(ptr);
    ARLib::printf("global op delete called on memory at %p with size %zu\n", ptr, size);
    std::free(ptr);
}
void operator delete[](void* ptr) {
    if (ptr == nullptr) return;
    auto size = s_alloc_map.remove(ptr);
    ARLib::printf("global op delete[] called on memory at %p with size %zu\n", ptr, size);
    std::free(ptr);
}
#endif