# CMakeList.txt : CMake project for mingw-build-demo, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# msvc-main-nobuild - This executable will not build due to unresolved import for aligned_malloc and aligned_free
add_executable (msvc-main-nobuild "msvc-main.c")
target_link_libraries(msvc-main-nobuild msvc-lib-nobuild)
target_include_directories(msvc-main-nobuild PRIVATE "${CMAKE_SOURCE_DIR}/msvc-lib/")


# msvc-main-bug
# This executable defines __imp__aligned_malloc and __imp__aligned_free but results in improper linkage in MSVC, causing a crash
# Three variations in msvc-bug[1-3].c
# In order to recognize the functions as imported functions, the __imp__ prefix *must* be present - this becomes an issue in the next example
add_executable (msvc-main-bug "msvc-main.c")
target_link_libraries(msvc-main-bug msvc-lib-bug)
target_include_directories(msvc-main-bug PRIVATE "${CMAKE_SOURCE_DIR}/msvc-lib/")


# msvc-main-bug-nobuild
# This executable defines aligned_malloc and aligned_free, but because the __imp__ prefix is missing, they're not recognized as imported functions and not linked
add_executable(msvc-main-bug-nobuild "msvc-main.c")
target_link_libraries(msvc-main-bug-nobuild msvc-lib-bug-nobuild)
target_include_directories(msvc-main-bug-nobuild PRIVATE "${CMAKE_SOURCE_DIR}/msvc-lib/")


# msvc-main-hack-fix
# This .... this I don't understand
add_executable (msvc-main-hack "msvc-main-hack.c")
target_link_libraries(msvc-main-hack msvc-lib-bug-hack)
target_include_directories(msvc-main-hack PRIVATE "${CMAKE_SOURCE_DIR}/msvc-lib/")