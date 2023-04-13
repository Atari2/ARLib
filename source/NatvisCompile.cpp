#include "Compat.hpp"
#ifdef COMPILER_MSVC
namespace ARLib {
// this file exists just to force re-compilation upon changing the natvis file.
// ARLib provides natvis support for the following classes
// - String
// - StringView
// - WString
// - WStringView
// - Path
// - Pair
// - UniquePtr
// - SharedPtr
// - Vector
// - Result
// - Optional
// - Array
// - Monostate
// - Variant
// - RefBox
// - List
// - SortedVector
// - Stack
// - SourceLocation
// - BackTrace
// - SSOVector
// - WeakPtr
// - Tree
// - Matrix
// - HashMap
// - Map
// - GenericView
// - {Reverse}{Const}Iterator
// - IteratorView
// - MapIterate/FilterIterate/ZipIterate/PairIterate/Enumerate
// - MapIterator/IfIterator/ZipIterator/PairIterator/LoopIterator
// - Mutex
// - UniqueLock
// - ScopedLock
// - LockGuard
// - Thread
// - Tuple
}
#endif