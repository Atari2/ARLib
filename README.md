# ARLib
A simple WIP C++20 implementation of standard containers and library features that aims to be compatible with MSVC/GCC/Clang on 64 bit Windows and Linux systems.

Note: the developer of this library is not responsible for any error caused by it. Everything is based on necessity as it is a personal project done mostly for fun. PR and issues are welcome but not guaranteed to be fixed/merged. Do so at your own discretion.

For now it relies on:
- `<compare>` for `std::strong_ordering`
- `<type_traits>` for `std::is_foo_constructible<T, Args...>`, `std::is_foo_assignable<T, Args...>`, `std::is_union<T>`, `std::is_enum<T>`
- `<cstdio>` for file and console I/O, and for some number -> string conversions.
- `<immintrin.h>`, `<intrin.h>` for vector instruction intrinsics.
- `<typeinfo>` for typeid(Cls).name()
- `<new>` global operator new/new[\]/delete/delete[\] because I'm not implementing my own malloc and for placement new.
- `<initializer_list>` for `std::initializer_list<T>`
- `<pthread.h>` on Linux, this dependency can be disabled with -DDISABLE_THREADING passed upon running CMake.

Conditionally (and only for testing and profiling purposes) it also relies on `<chrono>`, however that dependancy gets added only for the test runner and only when adding `-DPERF_TEST=true` upon CMake generation. 

This library has been compiled and tested with:
- MSVC 19.29.30037.0
- GCC 11
- Clang 12
- Mingw x64 with GCC 11
- clang-cl 12

## How to build

Simply run cmake with the preferred settings of your choice.
For a standard release linux build for example:
```
git clone https://github.com/Atari2/ARLib
cd ARLib
mkdir build
cd build
cmake -DCMAKE_C_COMPILER=clang-12 -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_BUILD_TYPE=Release -G "Ninja" ..
ninja
```

If you want to use `make` just remove the `-G "Ninja"` and call `make` after instead of `ninja`.

For a standard release windows build:
```
git clone https://github.com/Atari2/ARLib
cd ARLib
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

To add this library to your cmake project, simply use the following in your CMakeLists.txt
```
include(FetchContent)

FetchContent_Declare(
	ARLib
	GIT_REPOSITORY "https://github.com/Atari2/ARLib"
	GIT_TAG master
)

FetchContent_MakeAvailable(ARLib)
target_include_directories(YOUR_PROJECT PRIVATE ${ARLib_SOURCE_DIR})
target_link_libraries(YOUR_PROJECT PRIVATE ARLib)
```

This requires CMake at least version 3.15