cmake_minimum_required(VERSION 3.15)
project(ARLib VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ARLIB_SOURCE_FILES "")
set(ARLIB_COMPILE_DEFINITIONS "")
set(ARLIB_COMPILE_OPTIONS "")
set(ARLIB_COMPILE_WARNINGS "")
set(ARLIB_LINK_LIBRARIES ARLib_asm_ext)
set(ARLIB_LINK_OPTIONS "")

# detecting compilers
set(ON_CLANG false)
set(ON_GCC false)
set(ON_MSVC false)

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
elseif (UNIX)
	set(ON_LINUX true)
	set(ON_WINDOWS false)
	message(STATUS "Unix build")
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

file(GLOB LIB_SOURCE_FILES_CPP CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB LIB_SOURCE_FILES_H CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
list(APPEND ARLIB_SOURCE_FILES ${LIB_SOURCE_FILES_H} ${LIB_SOURCE_FILES_CPP})

file(GLOB_RECURSE GeneratedEnums "GeneratedEnums/*.h")
list(APPEND ARLIB_SOURCE_FILES GeneratedEnums)
if (ON_WINDOWS)
	set(pyexe "py")
else()
	set(pyexe "python3")
endif()

execute_process(
					COMMAND ${pyexe} ${CMAKE_CURRENT_SOURCE_DIR}/genenums.py ${CMAKE_CURRENT_SOURCE_DIR}
					COMMAND_ECHO STDOUT
				 )
add_library(ARLib ${ARLIB_SOURCE_FILES})

if (ARLIB_TOP_LEVEL)
	add_subdirectory(Tests)
	add_subdirectory(Playground)
endif()

if (PERF_TEST)
	message(STATUS "Enabling perfomance runner")
	add_subdirectory(PerformanceTests)
endif()

if (ON_MSVC OR (ON_CLANG AND ON_WINDOWS))
	add_subdirectory(ASM_MSVC)
	list(APPEND ARLIB_COMPILE_OPTIONS /std:c++latest /arch:AVX2 /permissive-)
	if (DEBUG_BUILD)
		list(APPEND ARLIB_COMPILE_OPTIONS /Oy-)
	else()
		list(APPEND ARLIB_COMPILE_OPTIONS /O2)
	endif()
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
	list(APPEND ARLIB_COMPILE_OPTIONS -std=c++20 -msse4.1 -mavx -fno-exceptions)
	if (DEBUG_BUILD AND NOT ON_WINDOWS)
		list(APPEND ARLIB_COMPILE_OPTIONS -fsanitize=address,undefined,leak)
		list(APPEND ARLIB_LINK_OPTIONS -fsanitize=address,undefined,leak)
		list(APPEND ARLIB_LINK_LIBRARIES -fsanitize=address,undefined,leak)
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
			-Wuseless-cast				# gcc only
			)
    elseif(ON_CLANG)
		list(APPEND ARLIB_COMPILE_WARNINGS
			-Wno-gnu-zero-variadic-macro-arguments
			)
	endif()
	list(APPEND ARLIB_COMPILE_OPTIONS ${ARLIB_COMPILE_WARNINGS})
endif()

target_link_options(ARLib PUBLIC ${ARLIB_LINK_OPTIONS})
target_link_libraries(ARLib PUBLIC ${ARLIB_LINK_LIBRARIES})
target_compile_options(ARLib PUBLIC ${ARLIB_COMPILE_OPTIONS})
target_compile_definitions(ARLib PUBLIC ${ARLIB_COMPILE_DEFINITIONS})

if (ARLIB_TOP_LEVEL)
	add_custom_target(
		RUN_TEST ALL
		ARLibTestSuite
		COMMENT "Running tests"
	)
endif()
if (PERF_TEST)
	add_custom_target(
		RUN_PERF ALL
		ARLibPerformanceSuite
		COMMENT "Running performance tests"
	)
endif()