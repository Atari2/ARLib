#pragma once
#include "Concepts.h"
#include "SourceLocation.h"

#ifdef DEBUG_NEW_DELETE
#define LOC loc
#else
#define LOC
#endif

namespace ARLib {

    enum class DeallocType { Single, Multiple };

    template <class T>
    T* allocate(SourceLocation LOC = SourceLocation::current()) {
        T* ptr = new T;
#ifdef DEBUG_NEW_DELETE
        ARLib::printf("Allocated %p from `%s` in %s [%u:%u]\n", ptr, loc.function_name(), loc.file_name(), loc.line(),
                      loc.column());
#endif
        return ptr;
    }

    template <class T>
    T* allocate(size_t count, SourceLocation LOC = SourceLocation::current()) {
        T* ptr = new T[count];
#ifdef DEBUG_NEW_DELETE
        ARLib::printf("Allocated %p with size %zu from `%s` in %s [%u:%u]\n", ptr, count, loc.function_name(),
                      loc.file_name(), loc.line(), loc.column());
#endif
        return ptr;
    }

    template <class T>
    T* allocate_emplace(void* storage, SourceLocation LOC = SourceLocation::current()) {
        HARD_ASSERT(storage != nullptr, "Storage for emplace new shouldn't be null")
        T* ptr = new (storage) T;
#ifdef DEBUG_NEW_DELETE
        ARLib::printf("Allocated %p (placement) from `%s` in %s [%u:%u]\n", ptr, loc.function_name(), loc.file_name(),
                      loc.line(), loc.column());
#endif
        return ptr;
    }

    template <class T, typename... Args>
    T* construct(Args... args, SourceLocation LOC = SourceLocation::current()) requires Constructible<T, Args...> {
        T* ptr = new T{args...};
#ifdef DEBUG_NEW_DELETE
        ARLib::printf("Constructed %p from `%s` in %s [%u:%u]\n", ptr, loc.function_name(), loc.file_name(), loc.line(),
                      loc.column());
#endif
        return ptr;
    }

    template <class T, typename... Args>
    T* construct_emplace(void* storage, Args... args, SourceLocation LOC = SourceLocation::current()) {
        T* ptr = new (storage) T{args...};
#ifdef DEBUG_NEW_DELETE
        ARLib::printf("Constructed %p (placement) from `%s` in %s [%u:%u]\n", ptr, loc.function_name(), loc.file_name(),
                      loc.line(), loc.column());
#endif
        return ptr;
    }

    template <class T, DeallocType D>
    void deallocate(T* allocated_ptr, SourceLocation LOC = SourceLocation::current()) {
        if (allocated_ptr == nullptr) return;
#ifdef DEBUG_NEW_DELETE
        if constexpr (D == DeallocType::Single)
            ARLib::printf("Deallocated %p from `%s` in %s [%u:%u]\n", allocated_ptr, loc.function_name(),
                          loc.file_name(), loc.line(), loc.column());
        else
            ARLib::printf("Deallocated %p (array) from `%s` in %s [%u:%u]\n", allocated_ptr, loc.function_name(),
                          loc.file_name(), loc.line(), loc.column());
#endif
        if constexpr (D == DeallocType::Single)
            delete allocated_ptr;
        else
            delete[] allocated_ptr;
    }

} // namespace ARLib