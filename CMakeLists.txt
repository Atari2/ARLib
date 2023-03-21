cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0117 NEW)
cmake_minimum_required(VERSION 3.20)
project(ARLib VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ARLIB_SOURCE_FILES "")
set(ARLIB_COMPILE_DEFINITIONS "")
set(ARLIB_COMPILE_OPTIONS "")
set(ARLIB_COMPILE_WARNINGS "")
set(ARLIB_LINK_LIBRARIES ARLib_asm_ext)
set(ARLIB_LINK_OPTIONS "")

option(ARLIB_FORCE_SANITIZERS "Always enable sanitizers, even in release builds" OFF)
option(ARLIB_DISABLE_SANITIZERS "Always disable sanitizers, this takes precendence over ARLIB_FORCE_SANITIZERS" OFF)
if (MSVC)
    option(MSVC_STATIC_STL "Use the static version of the MSVC STL to link" OFF)
    if (NOT MSVC_STATIC_STL)
        set(ARLIB_MSVC_LIB_TYPE "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        message(STATUS "Using DLL flavor of MSVC's STL")
    else()
        set(ARLIB_MSVC_LIB_TYPE "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        message(STATUS "Using LIB flavor of MSVC's STL")
    endif()
endif()

# detecting compilers
set(ON_CLANG false)
set(ON_GCC false)
set(ON_MSVC false)
if (DEFINED DISABLE_THREADING)
	list(APPEND ARLIB_COMPILE_DEFINITIONS DISABLE_THREADING)
endif()

if (${CMAKE_VERSION} VERSION_LESS "3.21")
	get_directory_property(hasParent PARENT_DIRECTORY)
	if (hasParent)
		message(STATUS "ARLib is not top level")
		set(ARLIB_TOP_LEVEL false)
	else()
		message(STATUS "ARLib is top level")
		set(ARLIB_TOP_LEVEL true)
	endif()
else()
	if (PROJECT_IS_TOP_LEVEL)
		message(STATUS "ARLib is top level")
		set(ARLIB_TOP_LEVEL true)
	else()
		message(STATUS "ARLib is not top level")
		set(ARLIB_TOP_LEVEL false)
	endif()
endif()

# detecting platform
if (WIN32)
	set(ON_WINDOWS true)
	set(ON_LINUX false)
	message(STATUS "Windows build")
	list(APPEND ARLIB_COMPILE_DEFINITIONS ON_WINDOWS)
elseif (UNIX)
	set(ON_LINUX true)
	set(ON_WINDOWS false)
	message(STATUS "Unix build")
	list(APPEND ARLIB_COMPILE_DEFINITIONS ON_LINUX)
	if (DEFINED DISABLE_THREADING)
		message(STATUS "Disabling threading will not link against pthread on Linux but will disable the use of Threading.h and similar")
	else()
		list(APPEND ARLIB_LINK_LIBRARIES pthread)
	endif()
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

if (DEBUG_NEW_DELETE)
	message(STATUS "Enabling debug new/delete functions")
	list(APPEND ARLIB_COMPILE_DEFINITIONS DEBUG_NEW_DELETE)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(ON_CLANG true)
	list(APPEND ARLIB_COMPILE_DEFINITIONS COMPILER_CLANG)
	message(STATUS "Using Clang/Clang++ compiler")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(ON_GCC true)
	list(APPEND ARLIB_COMPILE_DEFINITIONS COMPILER_GCC)
	message(STATUS "Using GCC/G++ compiler")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(ON_MSVC true)
	list(APPEND ARLIB_COMPILE_DEFINITIONS COMPILER_MSVC)
	message(STATUS "Using MSVC compiler")
else()
	message(FATAL_ERROR "Compiler unknown, only valid compilers are Clang/GCC/MSVC")
endif()

if (ON_WINDOWS AND ON_GCC)
	message(STATUS "Using MINGW for some reason")
	list(APPEND ARLIB_COMPILE_DEFINITIONS ON_MINGW)
endif()

#detecting build type
if (CMAKE_CONFIGURATION_TYPES STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	list(APPEND ARLIB_COMPILE_DEFINITIONS DEBUG)
	set(DEBUG_BUILD true)
	set(RELEASE_BUILD false)
	message(STATUS "In debug mode")
else()
	set(DEBUG_BUILD false)
	set(RELEASE_BUILD true)
	message(STATUS "In release mode")
endif()

# this is a bit confusing so let's explain
# on release builds we never enable sanitizers, unless explicitely asked for.
# on debug builds, we add them (asan on windows, asan+ubsan+leaksan on linux, 
#                               except when using clang-cl because clang-cl doesn't like asan in debug builds)
if ((DEBUG_BUILD OR ARLIB_FORCE_SANITIZERS) AND NOT DEBUG_NEW_DELETE)
    if (ON_WINDOWS AND ON_CLANG)
        message(STATUS "Clang doesn't like debug libraries + asan")
        set(ARLIB_SANITIZERS_FLAGS "")
    elseif(ON_LINUX)
        set(ARLIB_SANITIZERS_FLAGS "-fsanitize=address,undefined,leak")
    else()
        set(ARLIB_SANITIZERS_FLAGS "/fsanitize=address")
    endif()
else()
    set(ARLIB_SANITIZERS_FLAGS "")
endif()
if (ARLIB_DISABLE_SANITIZERS)
    set(ARLIB_SANITIZERS_FLAGS "")
endif()
message(STATUS "Setting sanitizers flags to \"${ARLIB_SANITIZERS_FLAGS}\"")

if (
	CMAKE_CONFIGURATION_TYPES STREQUAL "Debug" OR 
	CMAKE_BUILD_TYPE STREQUAL "Debug" OR 
	CMAKE_CONFIGURATION_TYPES STREQUAL "RelWithDebInfo" OR 
	CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"
)
	set(INCLUDE_DEBUG_INFO TRUE)
else()
	set(INCLUDE_DEBUG_INFO FALSE)
endif()

if (DEBUG_BUILD)
	if (ON_WINDOWS)
		list(APPEND ARLIB_LINK_LIBRARIES dbghelp)
	else()
		list(APPEND ARLIB_LINK_OPTIONS -rdynamic)
	endif()
endif()

if (ON_WINDOWS)
	set(pyexe "py")
else()
	set(pyexe "python3")
endif()

execute_process(
					COMMAND ${pyexe} ${CMAKE_CURRENT_SOURCE_DIR}/genenums.py ${CMAKE_CURRENT_SOURCE_DIR}
					COMMAND_ECHO STDOUT
				 )
include(CheckAvx.cmake)
CheckAvx()
list(APPEND LIB_SOURCE_FILES_CPP 
    ${CMAKE_CURRENT_SOURCE_DIR}/NatvisCompile.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Assertion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BigInt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CharConv.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugNewDelete.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EventLoop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/FileSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Hash.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/HashBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JSONObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JSONParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Matrix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Ordering.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/Path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PrintInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Process.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Random.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SourceLocation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StackTrace.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/String.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/StringView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThreadBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Threading.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TypeInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UniqueString.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cmath_compat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cstring_compat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CpuInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PrintfImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cstdio_compat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WString.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WStringView.cpp
)
list(APPEND LIB_SOURCE_FILES_H
    ${CMAKE_CURRENT_SOURCE_DIR}/std_includes.h
	${CMAKE_CURRENT_SOURCE_DIR}/AdvancedIterators.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Algorithm.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Allocator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ArgParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Array.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Assertion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Async.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Atomic.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Badge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BigInt.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BitInteger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CharConv.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CharConvHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Chrono.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CircularList.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Comparator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Compat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Concepts.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Console.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ContextManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Conversion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CpuInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/CxprHashMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DebugNewDelete.h
    ${CMAKE_CURRENT_SOURCE_DIR}/EnumHelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Enumerate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/EventLoop.h
    ${CMAKE_CURRENT_SOURCE_DIR}/File.h
	${CMAKE_CURRENT_SOURCE_DIR}/FileSystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FixedMatrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Functional.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Hash.h
    ${CMAKE_CURRENT_SOURCE_DIR}/HashBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/HashMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/HashTable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ImplProcessCommon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/IntrusiveMap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Invoke.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Iterator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/IteratorInspection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/JSONObject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/JSONParser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LinkedSet.h
    ${CMAKE_CURRENT_SOURCE_DIR}/List.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Macros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Map.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Matrix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Memory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/NumberTraits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Optional.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Ordering.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Pair.h
	${CMAKE_CURRENT_SOURCE_DIR}/Path.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PrintInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Printer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PrintfImpl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PriorityQueue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Process.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Random.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RefBox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Result.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SSOVector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Set.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SharedPtr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SortedVector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SourceLocation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Span.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Stack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/StackTrace.h
    ${CMAKE_CURRENT_SOURCE_DIR}/String.h
    ${CMAKE_CURRENT_SOURCE_DIR}/StringLiteral.h
    ${CMAKE_CURRENT_SOURCE_DIR}/StringView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Test.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ThreadBase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Threading.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Tree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Tuple.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TypeInfo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TypeTraits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UniquePtr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UniqueString.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Variant.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WeakPtr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/arlib_osapi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cmath_compat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cstdio_compat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cstring_compat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/cstdarg_compat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BaseTraits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GenericView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Vector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WString.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WStringView.h
)
list(APPEND LIB_SOURCE_FILES_XNATIVE
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/chrono/xnative_chrono_merge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/chrono/xnative_chrono_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/chrono/xnative_chrono_windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/chrono/xnative_chrono_merge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/chrono/xnative_chrono_unix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/chrono/xnative_chrono_windows.h

    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/thread/xnative_thread_merge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/thread/xnative_thread_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/thread/xnative_thread_windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/thread/xnative_thread_merge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/thread/xnative_thread_unix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/thread/xnative_thread_windows.h

	${CMAKE_CURRENT_SOURCE_DIR}/XNative/fs/xnative_fs_merge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/fs/xnative_fs_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/fs/xnative_fs_windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/fs/xnative_fs_merge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/fs/xnative_fs_unix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/fs/xnative_fs_windows.h

	${CMAKE_CURRENT_SOURCE_DIR}/XNative/atomic/xnative_atomic_merge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/atomic/xnative_atomic_unix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/atomic/xnative_atomic_windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/atomic/xnative_atomic_merge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/atomic/xnative_atomic_unix.h
    ${CMAKE_CURRENT_SOURCE_DIR}/XNative/atomic/xnative_atomic_windows.h
)
if (ON_WINDOWS)
    list(APPEND ARLIB_SOURCE_FILES 
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows/arlib_winapi.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows/win_native_io.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows/win_native_io.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows/win_native_structs.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows/win_native_structs.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows/win_native_process.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows/win_native_process.h
    )
else()
    list(APPEND ARLIB_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Linux/arlib_linuxapi.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Linux/linux_native_io.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Linux/linux_native_io.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Linux/linux_native_process.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Linux/linux_native_process.h
    )
endif()
list(APPEND ARLIB_SOURCE_FILES ${LIB_SOURCE_FILES_H} ${LIB_SOURCE_FILES_CPP})
list(APPEND ARLIB_SOURCE_FILES ${LIB_SOURCE_FILES_XNATIVE})

file(GLOB_RECURSE GeneratedEnums "GeneratedEnums/*.h")
list(APPEND ARLIB_SOURCE_FILES GeneratedEnums)

add_library(ARLib ${ARLIB_SOURCE_FILES})
set_property(TARGET ARLib PROPERTY MSVC_RUNTIME_LIBRARY ${ARLIB_MSVC_LIB_TYPE})

if (ON_WINDOWS)
    find_program(POWERSHELL_PATH NAMES powershell)
endif()

if (ARLIB_TOP_LEVEL)
	add_subdirectory(Tests)
	add_subdirectory(Playground)
	add_subdirectory(Benchmark)
endif()

if (DEBUG_BUILD)
    list(APPEND ARLIB_COMPILE_DEFINITIONS ARLIB_DEBUG)
endif()

if (ON_MSVC OR (ON_CLANG AND ON_WINDOWS))
	add_subdirectory(ASM_MSVC)
	list(APPEND ARLIB_COMPILE_OPTIONS /utf-8 /std:c++20 ${AVX_FLAGS} /permissive- /D_HAS_EXCEPTIONS=0 /EHsc)
    list(APPEND ARLIB_LINK_LIBRARIES synchronization winmm)
    if (MSVC AND NOT ON_CLANG)
        list(APPEND ARLIB_COMPILE_OPTIONS /Zc:inline-)
    endif()
	list(APPEND ARLIB_COMPILE_DEFINITIONS 
		_CRT_SECURE_NO_WARNINGS 
        UNICODE 
        _UNICODE
        WIN32_LEAN_AND_MEAN
        VC_EXTRALEAN
        NOMINMAX
	)
	if (DEBUG_BUILD)
		list(APPEND ARLIB_COMPILE_OPTIONS /DEBUG /Oy- ${ARLIB_SANITIZERS_FLAGS})
	else()
		list(APPEND ARLIB_COMPILE_OPTIONS /O2 /GR-)
	endif()
    list(APPEND ARLIB_LINK_OPTIONS "/NATVIS:${CMAKE_CURRENT_SOURCE_DIR}/ARLib.natvis")
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/NatvisCompile.cpp PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ARLib.natvis")
	list(APPEND ARLIB_COMPILE_WARNINGS
		/W3 
		/w14242 
		/w14254 
		/w14265 
		/w14287 
		/we4289 
		/w14296 
		/w14311 
		/w14545 
		/w14546 
		/w14547 
		/w14549 
		/w14555 
		/w14619 
		/w14640 
		/w14826 
		/w14905 
		/w14906 
		/w14928
		)
	if (ON_CLANG)
	list(APPEND ARLIB_COMPILE_WARNINGS
			-Wno-c++17-extensions
			-Wno-deprecated-declarations
		)
	endif()
	list(APPEND ARLIB_COMPILE_OPTIONS ${ARLIB_COMPILE_WARNINGS})
else()
	add_subdirectory(ASM_GCC)
	list(APPEND ARLIB_COMPILE_OPTIONS -std=c++20 -msse4.1 ${AVX_FLAGS} -fno-exceptions)
	if (DEBUG_BUILD AND NOT ON_WINDOWS)
		list(APPEND ARLIB_COMPILE_OPTIONS ${ARLIB_SANITIZERS_FLAGS})
		list(APPEND ARLIB_LINK_OPTIONS ${ARLIB_SANITIZERS_FLAGS})
		list(APPEND ARLIB_LINK_LIBRARIES ${ARLIB_SANITIZERS_FLAGS})
	endif()
	list(APPEND ARLIB_COMPILE_WARNINGS
		-Wall 
		-Wextra
		-Wpedantic
		-Wshadow 
		-Wnon-virtual-dtor
		-Wold-style-cast
		-Wcast-align
		-Wunused 
		-Woverloaded-virtual 
		-Wpedantic 
		-Wconversion 
		-Wsign-conversion 
		-Wmisleading-indentation
		-Wnull-dereference 
		-Wdouble-promotion
		-Wformat=2
		-Wno-format-nonliteral
	)
	if (ON_GCC)
		list(APPEND ARLIB_COMPILE_WARNINGS
			-Wduplicated-cond			# gcc only
			-Wduplicated-branches		# gcc only
			-Wlogical-op				# gcc only
			)
    elseif(ON_CLANG)
		list(APPEND ARLIB_COMPILE_WARNINGS
			-Wno-gnu-zero-variadic-macro-arguments
			-Wno-nested-anon-types
			)
	endif()
	list(APPEND ARLIB_COMPILE_OPTIONS ${ARLIB_COMPILE_WARNINGS})
endif()


if (INCLUDE_DEBUG_INFO)
	message(STATUS "Including debug info")
	if (ON_MSVC OR (ON_CLANG AND ON_WINDOWS))
		list(APPEND ARLIB_LINK_OPTIONS /PROFILE /DEBUG:FULL)
	else()
		list(APPEND ARLIB_COMPILE_OPTIONS -g)
	endif()
endif()


target_link_options(ARLib PUBLIC ${ARLIB_LINK_OPTIONS})
target_link_libraries(ARLib PUBLIC ${ARLIB_LINK_LIBRARIES})
target_compile_options(ARLib PUBLIC ${ARLIB_COMPILE_OPTIONS})
target_compile_definitions(ARLib PUBLIC ${ARLIB_COMPILE_DEFINITIONS})