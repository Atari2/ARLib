#pragma once
#include "Concepts.hpp"
#include "SourceLocation.hpp"
#include "Assertion.hpp"

#ifdef DEBUG_NEW_DELETE
    #include <cstdio>
    #define LOC loc
bool get_stop_collection();
#else
    #define LOC
#endif
namespace ARLib {

enum class DeallocType { Single, Multiple };
template <class T>
T* allocate_uninitialized(SourceLocation LOC = SourceLocation::current()) {
    T* ptr = static_cast<T*>(::operator new(sizeof(T)));
#ifdef DEBUG_NEW_DELETE
    if (!get_stop_collection()) {
        ::printf(
        "Allocated %p from `%s` in %s [%u:%u]\n", ptr, loc.function_name(), loc.file_name(), loc.line(), loc.column()
        );
    }
#endif
    return ptr;
}
template <class T>
T* allocate_uninitialized(size_t count, SourceLocation LOC = SourceLocation::current()) {
    T* ptr = static_cast<T*>(::operator new(count * sizeof(T)));
#ifdef DEBUG_NEW_DELETE
    if (!get_stop_collection()) {
        ::printf(
        "Allocated %p with size %zu from `%s` in %s [%u:%u]\n", static_cast<void*>(ptr), count, loc.function_name(),
        loc.file_name(), loc.line(), loc.column()
        );
    }
#endif
    return ptr;
}
template <class T>
T* allocate_initialized(SourceLocation LOC = SourceLocation::current()) {
    T* ptr = new T{};
#ifdef DEBUG_NEW_DELETE
    if (!get_stop_collection()) {
        ::printf(
        "Allocated %p from `%s` in %s [%u:%u]\n", ptr, loc.function_name(), loc.file_name(), loc.line(), loc.column()
        );
    }
#endif
    return ptr;
}
template <class T>
T* allocate_initialized(size_t count, SourceLocation LOC = SourceLocation::current()) {
    T* ptr = allocate_uninitialized<T>(count);
    ptr    = new (ptr) T[count]{};
#ifdef DEBUG_NEW_DELETE
    if (!get_stop_collection()) {
        ::printf(
        "Allocated %p with size %zu from `%s` in %s [%u:%u]\n", static_cast<void*>(ptr), count, loc.function_name(),
        loc.file_name(), loc.line(), loc.column()
        );
    }
#endif
    return ptr;
}
template <class T, DeallocType D>
void deallocate(T* allocated_ptr, SourceLocation LOC = SourceLocation::current()) {
    if (allocated_ptr == nullptr) return;
#ifdef DEBUG_NEW_DELETE
    if (!get_stop_collection()) {
        if constexpr (D == DeallocType::Single)
            ::printf(
            "Deallocated %p from `%s` in %s [%u:%u]\n", static_cast<void*>(allocated_ptr), loc.function_name(),
            loc.file_name(), loc.line(), loc.column()
            );
        else
            ::printf(
            "Deallocated %p (array) from `%s` in %s [%u:%u]\n", static_cast<void*>(allocated_ptr), loc.function_name(),
            loc.file_name(), loc.line(), loc.column()
            );
    }
#endif
    auto* ptr = const_cast<typename RemoveConst<T>::type*>(allocated_ptr);
    if constexpr (D == DeallocType::Single)
        delete ptr;
    else { 
        void* mem = static_cast<void*>(ptr);
        ::operator delete(mem);
    }
}
}    // namespace ARLib
