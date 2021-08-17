cmake_minimum_required(VERSION 3.15)
project(ARLibPerformanceSuite VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PERF_SOURCE_FILES "")

list(
	APPEND PERF_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/perf.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PerfTime.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PerfTime.cpp"
	)

if (CMAKE_CONFIGURATION_TYPES STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG_BUILD true)
	set(RELEASE_BUILD false)
else()
	set(DEBUG_BUILD false)
	set(RELEASE_BUILD true)
endif()

message(STATUS "Building performance runner")
add_executable(ARLibPerformanceSuite ${PERF_SOURCE_FILES})
if (WIN32 AND NOT CMAKE_COMPILER_IS_GNUCXX)
	target_compile_options(ARLibPerformanceSuite PRIVATE /std:c++latest /arch:AVX2)
else()
	if (WIN32)
		target_compile_options(ARLibPerformanceSuite PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
	else()
		if (DEBUG_BUILD)
			target_compile_options(ARLibPerformanceSuite PRIVATE -fsanitize=address,undefined,leak -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
			target_link_options(ARLibPerformanceSuite PRIVATE -fsanitize=address,undefined,leak)
			target_link_libraries(ARLibPerformanceSuite PRIVATE -fsanitize=address,undefined,leak)
		else()
			target_compile_options(ARLibPerformanceSuite PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
		endif()
	endif()
endif()
target_link_libraries(ARLibPerformanceSuite PRIVATE ARLib)