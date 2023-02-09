#include "Compat.h"
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
	
	// currently not supported but on TODO list:
	// - HashMap
	// - Map
	// - List
	// - Tuple
	// - {Reverse}{Const}Iterator
	// - GenericView
	// - IteratorView
	// - Matrix
	// - Stack
	// - SortedVector
	// - SSOVector
	// - Tree
	// - WeakPtr
	// - SourceLocation
	// - StackTrace
	// - Mutex
	// - UniqueLock
	// - ScopedLock
	// - LockGuard
	// - Thread
}
#endif