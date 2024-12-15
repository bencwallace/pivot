# cmake settings
cmake_minimum_required(VERSION 3.19.0)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
project(pivot VERSION 0.1.0 LANGUAGES C CXX)

# global settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# imports
include(CTest)

# options
option(SANITIZE "Enable sanitizers" OFF)
option(GRAPHVIZ_INCLUDE_PATH "Custom path to graphviz headers" OFF)
set(DIMS_UB 6 CACHE STRING "Upper bound on dimensions handled" FORCE)

# dependencies
find_package(CLI11 CONFIG REQUIRED)
find_package(Boost REQUIRED)

# core library
file(GLOB_RECURSE pivot_lib_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM pivot_lib_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_library(pivot ${pivot_lib_SRC})
target_include_directories(pivot PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/include")
target_include_directories(pivot PUBLIC ${Boost_INCLUDE_DIRS})

## graphviz
if (GRAPHVIZ_INCLUDE_PATH)
  target_include_directories(pivot PUBLIC ${GRAPHVIZ_INCLUDE_PATH})
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GVC REQUIRED IMPORTED_TARGET libgvc)
  pkg_check_modules(CGRAPH REQUIRED IMPORTED_TARGET libcgraph)
  target_include_directories(pivot PUBLIC ${GVC_INCLUDE_DIRS} ${CGRAPH_INCLUDE_DIRS})
endif()

target_compile_options(pivot
  PUBLIC
    -Wall -Wextra -Werror -Wpedantic
    $<$<CONFIG:Debug>:-O0 -g>
)

## sanitizer option
if(SANITIZE)
  target_compile_options(pivot
    PUBLIC
      -fsanitize=address,leak,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined
  )
  target_link_options(pivot
    PUBLIC
      -fsanitize=address,leak,undefined
  )
endif()

## upper bound on dimension
target_compile_definitions(pivot
  PUBLIC
    DIMS_UB=${DIMS_UB}
)

# executable
add_executable(pivot_exec "src/main.cpp")
set_target_properties(pivot_exec PROPERTIES OUTPUT_NAME pivot)
target_link_libraries(pivot_exec PUBLIC pivot PRIVATE CLI11::CLI11)

## not sure why, but this has to be included after declaring pivot_exec
target_compile_definitions(pivot
  PUBLIC
    $<$<CONFIG:Debug>:_GLIBCXX_DEBUG>
)

# tests
add_subdirectory(tests)
