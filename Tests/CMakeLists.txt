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

if (ENABLED_TIMING)
	list(
		APPEND TEST_SOURCE_FILES
		"${CMAKE_CURRENT_SOURCE_DIR}/PerfTime.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/PerfTime.cpp"
	)
endif()

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest"
    GIT_TAG        master
)

enable_testing()

include (GoogleTest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (CMAKE_CONFIGURATION_TYPES STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG_BUILD true)
	set(RELEASE_BUILD false)
else()
	set(DEBUG_BUILD false)
	set(RELEASE_BUILD true)
endif()

message(STATUS "Building test runner")
add_executable(ARLibTestSuite ${TEST_SOURCE_FILES})
if (WIN32 AND NOT CMAKE_COMPILER_IS_GNUCXX)
	target_compile_options(ARLibTestSuite PRIVATE /std:c++latest /arch:AVX2)
else()
	if (WIN32)
		target_compile_options(ARLibTestSuite PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
	else()
		if (DEBUG_BUILD)
			target_compile_options(ARLibTestSuite PRIVATE -fsanitize=address,undefined,leak -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
			target_link_options(ARLibTestSuite PRIVATE -fsanitize=address,undefined,leak)
			target_link_libraries(ARLibTestSuite PRIVATE -fsanitize=address,undefined,leak)
		else()
			target_compile_options(ARLibTestSuite PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
		endif()
	endif()
endif()
target_include_directories(ARLibTestSuite PRIVATE ${googletest_SOURCE_DIR})
target_link_libraries(ARLibTestSuite PRIVATE ARLib gtest_main)
gtest_discover_tests(ARLibTestSuite)