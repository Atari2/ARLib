cmake_minimum_required(VERSION 3.15)
project(ARLibPlayground VERSION 0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PLAYGROUND_SOURCE_FILES "")

CheckAvx()

list(
	APPEND PLAYGROUND_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
	)

message(STATUS "Building playground")
add_executable(ARLibPlayground ${PLAYGROUND_SOURCE_FILES})

if (MSVC)
	target_compile_options(ARLibPlayground PRIVATE /std:c++latest /utf-8 ${AVX_FLAGS})
	add_custom_command(
        TARGET ARLibPlayground
        POST_BUILD
        COMMAND mt.exe -manifest ${CMAKE_CURRENT_SOURCE_DIR}/../ARLib.manifest -outputresource:$<TARGET_FILE:ARLibPlayground>
        COMMENT "Applying manifest file"
    )
else()
	if (WIN32)
		target_compile_definitions(ARLibPlayground PRIVATE UNICODE _UNICODE)
		target_compile_options(ARLibPlayground PRIVATE -std=c++20 -msse4.1 ${AVX_FLAGS} -fno-exceptions -Wall -Wextra)
	else()
		if (DEBUG_BUILD)
			target_compile_options(ARLibPlayground PRIVATE -fsanitize=address,undefined,leak -std=c++20 -msse4.1 ${AVX_FLAGS} -fno-exceptions -Wall -Wextra)
			target_link_options(ARLibPlayground PRIVATE -fsanitize=address,undefined,leak)
			target_link_libraries(ARLibPlayground PRIVATE -fsanitize=address,undefined,leak)
		else()
			target_compile_options(ARLibPlayground PRIVATE -std=c++20 -msse4.1 ${AVX_FLAGS} -fno-exceptions -Wall -Wextra)
		endif()
	endif()
endif()
target_link_libraries(ARLibPlayground PRIVATE ARLib)