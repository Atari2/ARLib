cmake_minimum_required(VERSION 3.15)
project(ARLib_asm_ext VERSION 0.1)
set(ASM_LIB_SOURCE_FILES "")
list(
	APPEND ASM_LIB_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/exp.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/log.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/log10.cpp"
)
add_library(ARLib_asm_ext ${ASM_LIB_SOURCE_FILES})
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	message(STATUS "Building debug asm library")
	target_compile_options(ARLib_asm_ext PRIVATE -g -O0)
else()
	message(STATUS "Building release asm library")
	target_compile_options(ARLib_asm_ext PRIVATE -O2)
endif()