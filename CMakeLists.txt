cmake_minimum_required(VERSION 3.15)
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
	"${CMAKE_CURRENT_SOURCE_DIR}/Ordering.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CircularList.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Coroutine.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Functional.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Tuple.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Iterator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/String.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/String.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/StringView.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/StringView.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/StringProxy.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/StringProxy.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/BaseTraits.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/TypeTraits.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/SortedVector.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Vector.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Stack.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Test.h"
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
	"${CMAKE_CURRENT_SOURCE_DIR}/Result.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Algorithm.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Algorithm.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ArgParser.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CharConv.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Badge.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Pair.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/RefBox.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Enumerate.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/UniquePtr.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/SharedPtr.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CpuInfo.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CpuInfo.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/File.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/File.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Memory.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/GenericView.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/ContextManager.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/EnumHelpers.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/cstring_compat.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/cstring_compat.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/cstdio_compat.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/cstdio_compat.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmath_compat.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/cmath_compat.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/std_includes.h"
)

file(GLOB_RECURSE GeneratedEnums "GeneratedEnums/*.h")
list(APPEND LIB_SOURCE_FILES GeneratedEnums)
if (WIN32)
	set(pyexe "py")
else()
	set(pyexe "python3")
endif()

execute_process(
					COMMAND ${pyexe} ${CMAKE_CURRENT_SOURCE_DIR}/genenums.py ${CMAKE_CURRENT_SOURCE_DIR}
					COMMAND_ECHO STDOUT
				 )
add_library(ARLib ${LIB_SOURCE_FILES})
add_subdirectory(Tests)
if (WIN32 AND NOT CMAKE_COMPILER_IS_GNUCXX)
    add_subdirectory(ASM_MSVC)
	target_compile_options(ARLib PRIVATE /std:c++latest /arch:AVX2)
else()
	add_subdirectory(ASM_GCC)
	if (WIN32)
		if (CMAKE_BUILD_TYPE STREQUAL "Debug")
			target_compile_options(ARLib PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
		else()
			target_compile_options(ARLib PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra -Wno-unused-const-variable)
		endif()
	else()
		if (CMAKE_BUILD_TYPE STREQUAL "Debug")
			message(STATUS "Building debug lib")
			target_compile_options(ARLib PRIVATE -fsanitize=address,undefined,leak -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
			target_link_options(ARLib PRIVATE -fsanitize=address,undefined,leak)
			target_link_libraries(ARLib ARLib_asm_ext -fsanitize=address,undefined,leak)
		else()
			target_compile_options(ARLib PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra -Wno-unused-const-variable)
		endif()
	endif()
endif()
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug" OR WIN32)
	target_link_libraries(ARLib ARLib_asm_ext)
endif()
if (CMAKE_CONFIGURATION_TYPES STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(ARLib PUBLIC DEBUG)
endif()

add_custom_target(
	RUN_TEST ALL
	ARLibTestSuite
	COMMENT "Running tests"
)