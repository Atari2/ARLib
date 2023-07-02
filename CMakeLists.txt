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
option(ARLIB_USE_LIBCXX "Use libc++ as standard library (only available when compiling with clang, will be ignored otherwise)" OFF)
option(ARLIB_USE_MOLD "Use the mold linker instead of the standard linker (requires mold to be installed, available only on GCC and Clang)" OFF)
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
		message(STATUS "Disabling threading will not link against pthread on Linux but will disable the use of Threading.hpp and similar")
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


set(ARLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(ARLIB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source") 

list(APPEND LIB_SOURCE_FILES_CPP 
    ${ARLIB_SOURCE_DIR}/NatvisCompile.cpp
    ${ARLIB_SOURCE_DIR}/Algorithm.cpp
    ${ARLIB_SOURCE_DIR}/Assertion.cpp
    ${ARLIB_SOURCE_DIR}/BigInt.cpp
    ${ARLIB_SOURCE_DIR}/CharConv.cpp
    ${ARLIB_SOURCE_DIR}/DebugNewDelete.cpp
    ${ARLIB_SOURCE_DIR}/EventLoop.cpp
    ${ARLIB_SOURCE_DIR}/File.cpp
	${ARLIB_SOURCE_DIR}/FileSystem.cpp
    ${ARLIB_SOURCE_DIR}/FlatSet.cpp
    ${ARLIB_SOURCE_DIR}/Hash.cpp
    ${ARLIB_SOURCE_DIR}/HashBase.cpp
    ${ARLIB_SOURCE_DIR}/JSONObject.cpp
    ${ARLIB_SOURCE_DIR}/JSONParser.cpp
    ${ARLIB_SOURCE_DIR}/Matrix.cpp
    ${ARLIB_SOURCE_DIR}/Ordering.cpp
	${ARLIB_SOURCE_DIR}/Path.cpp
    ${ARLIB_SOURCE_DIR}/PrintInfo.cpp
    ${ARLIB_SOURCE_DIR}/Process.cpp
    ${ARLIB_SOURCE_DIR}/Random.cpp
    ${ARLIB_SOURCE_DIR}/Regex.cpp
    ${ARLIB_SOURCE_DIR}/SourceLocation.cpp
    ${ARLIB_SOURCE_DIR}/StackTrace.cpp
    ${ARLIB_SOURCE_DIR}/String.cpp
    ${ARLIB_SOURCE_DIR}/StringView.cpp
    ${ARLIB_SOURCE_DIR}/ThreadBase.cpp
    ${ARLIB_SOURCE_DIR}/Threading.cpp
    ${ARLIB_SOURCE_DIR}/TypeInfo.cpp
    ${ARLIB_SOURCE_DIR}/UniqueString.cpp
    ${ARLIB_SOURCE_DIR}/cmath_compat.cpp
    ${ARLIB_SOURCE_DIR}/cstring_compat.cpp
    ${ARLIB_SOURCE_DIR}/ArgParser.cpp
    ${ARLIB_SOURCE_DIR}/CpuInfo.cpp
    ${ARLIB_SOURCE_DIR}/PrintfImpl.cpp
    ${ARLIB_SOURCE_DIR}/cstdio_compat.cpp
    ${ARLIB_SOURCE_DIR}/WString.cpp
    ${ARLIB_SOURCE_DIR}/WStringView.cpp
)
list(APPEND LIB_SOURCE_FILES_H
    ${ARLIB_INCLUDE_DIR}/std_includes.hpp
	${ARLIB_INCLUDE_DIR}/AdvancedIterators.hpp
    ${ARLIB_INCLUDE_DIR}/Algorithm.hpp
    ${ARLIB_INCLUDE_DIR}/Allocator.hpp
    ${ARLIB_INCLUDE_DIR}/ArgParser.hpp
    ${ARLIB_INCLUDE_DIR}/Array.hpp
    ${ARLIB_INCLUDE_DIR}/Assertion.hpp
    ${ARLIB_INCLUDE_DIR}/Async.hpp
    ${ARLIB_INCLUDE_DIR}/Atomic.hpp
    ${ARLIB_INCLUDE_DIR}/Badge.hpp
    ${ARLIB_INCLUDE_DIR}/BigInt.hpp
    ${ARLIB_INCLUDE_DIR}/BitInteger.hpp
    ${ARLIB_INCLUDE_DIR}/CharConv.hpp
    ${ARLIB_INCLUDE_DIR}/CharConvHelpers.hpp
    ${ARLIB_INCLUDE_DIR}/Chrono.hpp
    ${ARLIB_INCLUDE_DIR}/CircularList.hpp
    ${ARLIB_INCLUDE_DIR}/Comparator.hpp
    ${ARLIB_INCLUDE_DIR}/Compat.hpp
    ${ARLIB_INCLUDE_DIR}/Concepts.hpp
    ${ARLIB_INCLUDE_DIR}/Console.hpp
    ${ARLIB_INCLUDE_DIR}/ContextManager.hpp
    ${ARLIB_INCLUDE_DIR}/Conversion.hpp
    ${ARLIB_INCLUDE_DIR}/CpuInfo.hpp
    ${ARLIB_INCLUDE_DIR}/CxprHashMap.hpp
    ${ARLIB_INCLUDE_DIR}/DebugNewDelete.hpp
    ${ARLIB_INCLUDE_DIR}/EnumHelpers.hpp
    ${ARLIB_INCLUDE_DIR}/Enumerate.hpp
    ${ARLIB_INCLUDE_DIR}/EventLoop.hpp
    ${ARLIB_INCLUDE_DIR}/File.hpp
	${ARLIB_INCLUDE_DIR}/FileSystem.hpp
    ${ARLIB_INCLUDE_DIR}/FixedMatrix.hpp
    ${ARLIB_INCLUDE_DIR}/FlatMap.hpp
    ${ARLIB_INCLUDE_DIR}/FlatSet.hpp
    ${ARLIB_INCLUDE_DIR}/Functional.hpp
    ${ARLIB_INCLUDE_DIR}/Hash.hpp
    ${ARLIB_INCLUDE_DIR}/HashBase.hpp
    ${ARLIB_INCLUDE_DIR}/ImplProcessCommon.hpp
    ${ARLIB_INCLUDE_DIR}/IntrusiveMap.hpp
    ${ARLIB_INCLUDE_DIR}/Invoke.hpp
    ${ARLIB_INCLUDE_DIR}/Iterator.hpp
    ${ARLIB_INCLUDE_DIR}/IteratorInspection.hpp
    ${ARLIB_INCLUDE_DIR}/JSONObject.hpp
    ${ARLIB_INCLUDE_DIR}/JSONParser.hpp
    ${ARLIB_INCLUDE_DIR}/LinkedSet.hpp
    ${ARLIB_INCLUDE_DIR}/List.hpp
    ${ARLIB_INCLUDE_DIR}/Macros.hpp
    ${ARLIB_INCLUDE_DIR}/Map.hpp
    ${ARLIB_INCLUDE_DIR}/Matrix.hpp
    ${ARLIB_INCLUDE_DIR}/Memory.hpp
    ${ARLIB_INCLUDE_DIR}/NumberTraits.hpp
    ${ARLIB_INCLUDE_DIR}/Optional.hpp
    ${ARLIB_INCLUDE_DIR}/Ordering.hpp
    ${ARLIB_INCLUDE_DIR}/Pair.hpp
	${ARLIB_INCLUDE_DIR}/Path.hpp
    ${ARLIB_INCLUDE_DIR}/PrintInfo.hpp
    ${ARLIB_INCLUDE_DIR}/Printer.hpp
    ${ARLIB_INCLUDE_DIR}/PrintfImpl.hpp
    ${ARLIB_INCLUDE_DIR}/PriorityQueue.hpp
    ${ARLIB_INCLUDE_DIR}/Process.hpp
    ${ARLIB_INCLUDE_DIR}/Random.hpp
    ${ARLIB_INCLUDE_DIR}/RefBox.hpp
    ${ARLIB_INCLUDE_DIR}/Regex.hpp
    ${ARLIB_INCLUDE_DIR}/Result.hpp
    ${ARLIB_INCLUDE_DIR}/SSOVector.hpp
    ${ARLIB_INCLUDE_DIR}/Set.hpp
    ${ARLIB_INCLUDE_DIR}/SharedPtr.hpp
    ${ARLIB_INCLUDE_DIR}/SortedVector.hpp
    ${ARLIB_INCLUDE_DIR}/SourceLocation.hpp
    ${ARLIB_INCLUDE_DIR}/Span.hpp
    ${ARLIB_INCLUDE_DIR}/Stack.hpp
    ${ARLIB_INCLUDE_DIR}/StackTrace.hpp
    ${ARLIB_INCLUDE_DIR}/String.hpp
    ${ARLIB_INCLUDE_DIR}/StringLiteral.hpp
    ${ARLIB_INCLUDE_DIR}/StringView.hpp
    ${ARLIB_INCLUDE_DIR}/Test.hpp
    ${ARLIB_INCLUDE_DIR}/ThreadBase.hpp
    ${ARLIB_INCLUDE_DIR}/Threading.hpp
    ${ARLIB_INCLUDE_DIR}/Tree.hpp
    ${ARLIB_INCLUDE_DIR}/Tuple.hpp
    ${ARLIB_INCLUDE_DIR}/TypeInfo.hpp
    ${ARLIB_INCLUDE_DIR}/TypeTraits.hpp
    ${ARLIB_INCLUDE_DIR}/Types.hpp
    ${ARLIB_INCLUDE_DIR}/UniquePtr.hpp
    ${ARLIB_INCLUDE_DIR}/UniqueString.hpp
    ${ARLIB_INCLUDE_DIR}/Variant.hpp
    ${ARLIB_INCLUDE_DIR}/WeakPtr.hpp
    ${ARLIB_INCLUDE_DIR}/arlib_osapi.hpp
    ${ARLIB_INCLUDE_DIR}/cmath_compat.hpp
    ${ARLIB_INCLUDE_DIR}/cstdio_compat.hpp
    ${ARLIB_INCLUDE_DIR}/cstring_compat.hpp
    ${ARLIB_INCLUDE_DIR}/cstdarg_compat.hpp
    ${ARLIB_INCLUDE_DIR}/BaseTraits.hpp
    ${ARLIB_INCLUDE_DIR}/GenericView.hpp
    ${ARLIB_INCLUDE_DIR}/Utility.hpp
    ${ARLIB_INCLUDE_DIR}/Vector.hpp
    ${ARLIB_INCLUDE_DIR}/WString.hpp
    ${ARLIB_INCLUDE_DIR}/WStringView.hpp
)
list(APPEND LIB_SOURCE_FILES_XNATIVE
    ${ARLIB_SOURCE_DIR}/XNative/chrono/xnative_chrono_merge.cpp
    ${ARLIB_SOURCE_DIR}/XNative/chrono/xnative_chrono_unix.cpp
    ${ARLIB_SOURCE_DIR}/XNative/chrono/xnative_chrono_windows.cpp
    ${ARLIB_INCLUDE_DIR}/XNative/chrono/xnative_chrono_merge.hpp
    ${ARLIB_INCLUDE_DIR}/XNative/chrono/xnative_chrono_unix.hpp
    ${ARLIB_INCLUDE_DIR}/XNative/chrono/xnative_chrono_windows.hpp

    ${ARLIB_SOURCE_DIR}/XNative/thread/xnative_thread_merge.cpp
    ${ARLIB_SOURCE_DIR}/XNative/thread/xnative_thread_unix.cpp
    ${ARLIB_SOURCE_DIR}/XNative/thread/xnative_thread_windows.cpp
    ${ARLIB_INCLUDE_DIR}/XNative/thread/xnative_thread_merge.hpp
    ${ARLIB_INCLUDE_DIR}/XNative/thread/xnative_thread_unix.hpp
    ${ARLIB_INCLUDE_DIR}/XNative/thread/xnative_thread_windows.hpp

	${ARLIB_SOURCE_DIR}/XNative/fs/xnative_fs_merge.cpp
    ${ARLIB_SOURCE_DIR}/XNative/fs/xnative_fs_unix.cpp
    ${ARLIB_SOURCE_DIR}/XNative/fs/xnative_fs_windows.cpp
    ${ARLIB_INCLUDE_DIR}/XNative/fs/xnative_fs_merge.hpp
    ${ARLIB_INCLUDE_DIR}/XNative/fs/xnative_fs_unix.hpp
    ${ARLIB_INCLUDE_DIR}/XNative/fs/xnative_fs_windows.hpp

	${ARLIB_SOURCE_DIR}/XNative/atomic/xnative_atomic_merge.cpp
    ${ARLIB_SOURCE_DIR}/XNative/atomic/xnative_atomic_unix.cpp
    ${ARLIB_SOURCE_DIR}/XNative/atomic/xnative_atomic_windows.cpp
    ${ARLIB_INCLUDE_DIR}/XNative/atomic/xnative_atomic_merge.hpp
    ${ARLIB_INCLUDE_DIR}/XNative/atomic/xnative_atomic_unix.hpp
    ${ARLIB_INCLUDE_DIR}/XNative/atomic/xnative_atomic_windows.hpp
)
if (ON_WINDOWS)
    list(APPEND ARLIB_SOURCE_FILES 
        ${ARLIB_SOURCE_DIR}/Windows/arlib_winapi.cpp
        ${ARLIB_SOURCE_DIR}/Windows/win_native_io.cpp
        ${ARLIB_INCLUDE_DIR}/Windows/win_native_io.hpp
        ${ARLIB_SOURCE_DIR}/Windows/win_native_structs.cpp
        ${ARLIB_INCLUDE_DIR}/Windows/win_native_structs.hpp
        ${ARLIB_SOURCE_DIR}/Windows/win_native_process.cpp
        ${ARLIB_INCLUDE_DIR}/Windows/win_native_process.hpp
    )
else()
    list(APPEND ARLIB_SOURCE_FILES
        ${ARLIB_SOURCE_DIR}/Linux/arlib_linuxapi.cpp
        ${ARLIB_SOURCE_DIR}/Linux/linux_native_io.cpp
        ${ARLIB_INCLUDE_DIR}/Linux/linux_native_io.hpp
        ${ARLIB_SOURCE_DIR}/Linux/linux_native_process.cpp
        ${ARLIB_INCLUDE_DIR}/Linux/linux_native_process.hpp
    )
endif()
list(APPEND ARLIB_SOURCE_FILES ${LIB_SOURCE_FILES_H} ${LIB_SOURCE_FILES_CPP})
list(APPEND ARLIB_SOURCE_FILES ${LIB_SOURCE_FILES_XNATIVE})

file(GLOB_RECURSE GeneratedEnums "${ARLIB_INCLUDE_DIR}/GeneratedEnums/*.hpp")
list(APPEND ARLIB_SOURCE_FILES ${GeneratedEnums})

add_library(ARLib ${ARLIB_SOURCE_FILES})
set_property(TARGET ARLib PROPERTY MSVC_RUNTIME_LIBRARY ${ARLIB_MSVC_LIB_TYPE})
target_include_directories(ARLib PUBLIC ${ARLIB_INCLUDE_DIR})

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
	list(APPEND ARLIB_COMPILE_OPTIONS /utf-8 ${AVX_FLAGS} /permissive- /D_HAS_EXCEPTIONS=0 /EHsc)
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
	list(APPEND ARLIB_COMPILE_OPTIONS -msse4.1 ${AVX_FLAGS} -fno-exceptions)
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
    elseif(ON_CLANG AND ARLIB_USE_LIBCXX)
        message(STATUS "Using libc++ instead of libstdc++")
        list(APPEND ARLIB_COMPILE_OPTIONS
            -stdlib=libc++
        )
        list(APPEND ARLIB_LINK_LIBRARIES
            -lc++abi
            -lc++
        )
        list(APPEND ARLIB_COMPILE_WARNINGS
			-Wno-gnu-zero-variadic-macro-arguments
			-Wno-nested-anon-types
		)
    elseif(ON_CLANG)
    	list(APPEND ARLIB_COMPILE_WARNINGS
			-Wno-gnu-zero-variadic-macro-arguments
			-Wno-nested-anon-types
		)
	endif()
    if (ARLIB_USE_MOLD)
        list(APPEND ARLIB_LINK_OPTIONS
            -fuse-ld=mold
        )
    elseif(ON_CLANG AND ARLIB_USE_LIBCXX)
        list(APPEND ARLIB_LINK_OPTIONS
            -fuse-ld=lld
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