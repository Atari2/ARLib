# ARLib
A simple WIP C++20 implementation of standard containers and library features that aimes to be compatible with MSVC/GCC/Clang on 64 bit Windows and Linux systems.

Note: the developer of this library is not responsible for any error caused by it. Everything is based on necessity as it is a personal project done mostly for fun. PR and issues are welcome but not guaranteed to be fixed/merged. Do so at your own discretion.

For now it relies on:
- initializer_list
- type_traits
- cstdio
- immintrin, intrin
- global operator new()

This library has been compiled and tested with:
- MSVC 19.29.30037.0
- GCC 11
- Clang 12
- Mingw x64 with GCC 11