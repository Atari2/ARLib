cmake_minimum_required(VERSION 3.10)
project(ARLibTestSuite VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(TEST_SOURCE_FILES "")

list(
	APPEND TEST_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Suite.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Suite.cpp"
	)

add_library(ARLibTestSuite ${TEST_SOURCE_FILES})
if (WIN32 AND NOT CMAKE_COMPILER_IS_GNUCXX)
	target_compile_options(ARLibTestSuite PRIVATE /std:c++latest /arch:AVX2)
else()
	if (WIN32)
		target_compile_options(ARLibTestSuite PRIVATE -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
	else()
		target_compile_options(ARLibTestSuite PRIVATE -fsanitize=address,undefined,leak -std=c++20 -msse4.1 -mavx -fno-exceptions -Wall -Wextra)
		target_link_options(ARLibTestSuite PRIVATE -fsanitize=address,undefined,leak)
		target_link_libraries(ARLibTestSuite ARLib_asm_ext -fsanitize=address,undefined,leak)
	endif()
endif()