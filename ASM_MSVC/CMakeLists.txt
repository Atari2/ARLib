cmake_minimum_required(VERSION 3.15)
project(ARLib_asm_ext VERSION 0.1)
enable_language(ASM_MASM)
set(ASM_LIB_SOURCE_FILES "")
list(
	APPEND ASM_LIB_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/exp.asm"
	"${CMAKE_CURRENT_SOURCE_DIR}/log.asm"
	"${CMAKE_CURRENT_SOURCE_DIR}/log10.asm"
)
set(CMAKE_ASM_MASM_CREATE_STATIC_LIBRARY "<CMAKE_AR> /OUT:<TARGET> <LINK_FLAGS> <OBJECTS>")		# placeholder to make it compile I guess
add_library(ARLib_asm_ext ${ASM_LIB_SOURCE_FILES})
target_compile_options(ARLib_asm_ext PRIVATE /Zd)