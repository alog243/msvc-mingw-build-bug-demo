# Add MinGW-built library with imported functions from stdlib
add_library(mingw-lib STATIC IMPORTED)
set_target_properties(mingw-lib PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/mingw-lib/mingw-lib.a")

# Add MSVC-built libraries

# msvc-lib-nobuild
# This will not build because of missing references to __imp__aligned_malloc and __imp__aligned_free
add_library(msvc-lib-nobuild STATIC "msvc-lib.c")
target_include_directories(msvc-lib-nobuild PRIVATE "${CMAKE_SOURCE_DIR}/mingw-lib/")
target_link_libraries(msvc-lib-nobuild mingw-lib)

# msvc-lib-bug[1-3] - all 3 files behave similarly so only one is built for brevity
# These will all build but crash because the linker improperly links the __imp__ functions
# Here's what I've found:
#	If the function is defined with the __imp__ prefix, the linker will improperly link these functions
#	If the function is NOT defined with the __imp__ prefix, they will properly work and not crash
add_library(msvc-lib-bug STATIC "msvc-lib.c" "msvc-bug2.c")
target_include_directories(msvc-lib-bug PRIVATE "${CMAKE_SOURCE_DIR}/mingw-lib/")
target_link_libraries(msvc-lib-bug mingw-lib)

# msvc-lib-bug4-nobuild
# This is where it gets weird. If NEITHER of the two functions are defined with the __imp__ prefix, they're not even recognized and linked, and it won't build
add_library(msvc-lib-bug-nobuild STATIC "msvc-lib.c" "msvc-bug4-nobuild.c")
target_include_directories(msvc-lib-bug-nobuild PRIVATE "${CMAKE_SOURCE_DIR}/mingw-lib/")
target_link_libraries(msvc-lib-bug-nobuild mingw-lib)

# msvc-lib-bug5-hack
# This ... this I don't understand.
# It seems like calling a function (ANY function) from an object file that contains a reference to one of the exposed __imp__ functions (even if never used in any code-path!)
# causes the other non-__imp__ functions to be recognized as imported functions and properly linked.
add_library(msvc-lib-bug-hack STATIC "msvc-lib.c" "msvc-bug5-hack.c" "msvc-bug5-extrafile.c")
target_include_directories(msvc-lib-bug-hack PRIVATE "${CMAKE_SOURCE_DIR}/mingw-lib/")
target_link_libraries(msvc-lib-bug-hack mingw-lib)