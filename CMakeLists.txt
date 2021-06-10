cmake_minimum_required(VERSION 3.10)
project(ARLib VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(LIB_SOURCE_FILES "")
list(
	APPEND LIB_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Types.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Utility.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Concepts.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Ordering.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CircularList.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Iterator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/String.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/String.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/StringView.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/BaseTraits.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/TypeTraits.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Vector.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Array.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Macros.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Assertion.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Assertion.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Compat.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/HashBase.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Set.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/HashTable.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Map.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/HashMap.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Optional.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Algorithm.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CharConv.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Badge.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/UniquePtr.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/SharedPtr.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CpuInfo.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CpuInfo.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/GenericView.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/cstring_compat.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/cstring_compat.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/cstdio_compat.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/cstdio_compat.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmath_compat.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmath_compat.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/std_includes.h"
)
add_library(ARLib ${LIB_SOURCE_FILES})
if (WIN32 AND NOT CMAKE_COMPILER_IS_GNUCXX)
    add_subdirectory(ASM_MSVC)
	target_compile_options(ARLib PRIVATE /std:c++latest /arch:AVX2)
else()
	add_subdirectory(ASM_GCC)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(ARLib PRIVATE -fsanitize=address,undefined,leak -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
		target_link_options(ARLib PRIVATE -fsanitize=address,undefined,leak)
	else()
		target_compile_options(ARLib PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra -Wno-unused-const-variable)
	endif()
endif()
target_link_libraries(ARLib ARLib_asm_ext)
if (CMAKE_CONFIGURATION_TYPES STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(ARLib PUBLIC DEBUG)
endif()