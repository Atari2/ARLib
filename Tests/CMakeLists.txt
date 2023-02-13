cmake_minimum_required(VERSION 3.20)
project(ARLibTestSuite VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(TEST_SOURCE_FILES "")

list(
	APPEND TEST_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Suite.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Suite.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Runner.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/GTestPrintHelpers.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/GTestPrintHelpers.cpp"
	)
	
if (CMAKE_CONFIGURATION_TYPES STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG_BUILD true)
	set(RELEASE_BUILD false)
else()
	set(DEBUG_BUILD false)
	set(RELEASE_BUILD true)
endif()

if (MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARLIB_SANITIZERS_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARLIB_SANITIZERS_FLAGS}")
endif()

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest"
    GIT_TAG        main
)

enable_testing()

include (GoogleTest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
set_property(TARGET gtest PROPERTY MSVC_RUNTIME_LIBRARY ${ARLIB_MSVC_LIB_TYPE})
set_property(TARGET gtest_main PROPERTY MSVC_RUNTIME_LIBRARY ${ARLIB_MSVC_LIB_TYPE})

CheckAvx()

message(STATUS "Building test runner")
add_executable(ARLibTestSuite ${TEST_SOURCE_FILES})
if (MSVC)
	set_property(TARGET ARLibTestSuite PROPERTY MSVC_RUNTIME_LIBRARY ${ARLIB_MSVC_LIB_TYPE})
	target_compile_options(ARLibTestSuite PRIVATE /std:c++20 ${AVX_FLAGS} /EHsc ${ARLIB_SANITIZERS_FLAGS})
else()
	set(ARLIB_DEFAULT_COMPILE_OPTIONS -std=c++20 -msse4.1 ${AVX_FLAGS} -fno-exceptions -Wall -Wextra)
	if (WIN32)
		target_compile_options(ARLibTestSuite PRIVATE ${ARLIB_DEFAULT_COMPILE_OPTIONS})
	else()
		if (DEBUG_BUILD)
			target_compile_options(ARLibTestSuite PRIVATE ${ARLIB_SANITIZERS_FLAGS} ${ARLIB_DEFAULT_COMPILE_OPTIONS})
			target_link_options(ARLibTestSuite PRIVATE ${ARLIB_SANITIZERS_FLAGS})
			target_link_libraries(ARLibTestSuite PRIVATE ${ARLIB_SANITIZERS_FLAGS})
		else()
			target_compile_options(ARLibTestSuite PRIVATE ${ARLIB_DEFAULT_COMPILE_OPTIONS})
		endif()
	endif()
endif()
if (ENABLED_TIMING)
	target_compile_definitions(ARLibTestSuite PRIVATE ENABLED_TIMING)
endif()
target_include_directories(ARLibTestSuite PRIVATE ${googletest_SOURCE_DIR})
target_link_libraries(ARLibTestSuite PRIVATE ARLib gtest_main)
gtest_discover_tests(ARLibTestSuite)
if (ON_MSVC OR (ON_CLANG AND ON_WINDOWS))
    add_custom_command(
		TARGET ARLibTestSuite POST_BUILD
		COMMAND ${POWERSHELL_PATH} -ExecutionPolicy Bypass -File ${CMAKE_CURRENT_SOURCE_DIR}/../copy_asan_dll.ps1 -TargetPath $<TARGET_FILE:ARLibTestSuite> -TargetDir $<TARGET_FILE_DIR:ARLibTestSuite>
		USES_TERMINAL
		VERBATIM
	)
endif()