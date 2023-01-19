cmake_minimum_required(VERSION 3.15)
project(ARLibTestSuite VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(TEST_SOURCE_FILES "")

list(
	APPEND TEST_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Suite.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Suite.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Runner.cpp"
	)
	
if (CMAKE_CONFIGURATION_TYPES STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG_BUILD true)
	set(RELEASE_BUILD false)
else()
	set(DEBUG_BUILD false)
	set(RELEASE_BUILD true)
endif()

if (MSVC AND DEBUG_BUILD)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fsanitize=address")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
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


CheckAvx()

message(STATUS "Building test runner")
add_executable(ARLibTestSuite ${TEST_SOURCE_FILES})
if (MSVC)
	if (DEBUG_BUILD)
		target_compile_options(ARLibTestSuite PRIVATE /std:c++latest ${AVX_FLAGS} /EHsc /fsanitize=address)
	else()
		target_compile_options(ARLibTestSuite PRIVATE /std:c++latest ${AVX_FLAGS} /EHsc)
	endif()
else()
	set(ARLIB_DEFAULT_COMPILE_OPTIONS -std=c++20 -msse4.1 ${AVX_FLAGS} -fno-exceptions -Wall -Wextra)
	if (WIN32)
		target_compile_options(ARLibTestSuite PRIVATE ${ARLIB_DEFAULT_COMPILE_OPTIONS})
	else()
		if (DEBUG_BUILD)
			target_compile_options(ARLibTestSuite PRIVATE -fsanitize=address,undefined,leak ${ARLIB_DEFAULT_COMPILE_OPTIONS})
			target_link_options(ARLibTestSuite PRIVATE -fsanitize=address,undefined,leak)
			target_link_libraries(ARLibTestSuite PRIVATE -fsanitize=address,undefined,leak)
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