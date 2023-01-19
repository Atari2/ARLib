cmake_minimum_required(VERSION 3.15)
project(ARLibBenchmarkSuite VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(TEST_SOURCE_FILES "")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
set(BENCHMARK_ENABLE_TESTING OFF)

list(
	APPEND TEST_SOURCE_FILES
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
    googlebenchmark
    GIT_REPOSITORY "https://github.com/google/benchmark"
    GIT_TAG        main
)

FetchContent_MakeAvailable(googlebenchmark)

message(STATUS "Building benchmark runner")
add_executable(ARLibBenchmarkSuite ${TEST_SOURCE_FILES})
if (WIN32 AND NOT CMAKE_COMPILER_IS_GNUCXX)
	if (DEBUG_BUILD)
		target_compile_options(ARLibTestSuite PRIVATE /std:c++latest ${AVX_FLAGS} /EHsc /fsanitize=address)
	else()
		target_compile_options(ARLibTestSuite PRIVATE /std:c++latest ${AVX_FLAGS} /EHsc)
	endif()
else()
	if (WIN32)
		target_compile_options(ARLibBenchmarkSuite PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
	else()
		if (DEBUG_BUILD)
			target_compile_options(ARLibBenchmarkSuite PRIVATE -fsanitize=address,undefined,leak -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
			target_link_options(ARLibBenchmarkSuite PRIVATE -fsanitize=address,undefined,leak)
			target_link_libraries(ARLibBenchmarkSuite PRIVATE -fsanitize=address,undefined,leak)
		else()
			target_compile_options(ARLibBenchmarkSuite PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
		endif()
	endif()
endif()
target_include_directories(ARLibBenchmarkSuite PRIVATE ${benchmark_SOURCE_DIR})
target_link_libraries(ARLibBenchmarkSuite PRIVATE ARLib benchmark::benchmark benchmark::benchmark_main)