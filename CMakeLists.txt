# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.18)

project (many_lights)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include(assets.cmake)

# Include sub-projects.
add_subdirectory(many_lights)
add_subdirectory(test_application)
add_subdirectory(third_party)
