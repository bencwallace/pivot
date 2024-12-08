cmake_minimum_required(VERSION 3.19.0)
project(pivot VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic" CACHE STRING "Flags used by the compiler during all builds." FORCE)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -D_GLIBCXX_DEBUG" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
option(SANITIZE "Enable sanitizers" OFF)
if(SANITIZE)
  set(SANITIZE_FLAGS "-fsanitize=address,leak,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZE_FLAGS}")
endif()

include(CTest)

include(FetchContent)
FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG ccd6801
)
FetchContent_MakeAvailable(cli11_proj)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

option(GRAPHVIZ_INCLUDE_PATH "Path to graphviz headers" OFF)
if(NOT GRAPHVIZ_INCLUDE_PATH)
  set(GRAPHVIZ_INCLUDE_PATH "/usr/include/graphviz" CACHE STRING "Default path to graphviz headers" FORCE)
endif()
include_directories(${GRAPHVIZ_INCLUDE_PATH})

option(DIMS_UB "Upper bound on dimensions handled" OFF)
if(NOT DIMS_UB)
  set(DIMS_UB 6 CACHE STRING "Default upper bound on dimensions handled" FORCE)
endif()
add_definitions(-DDIMS_UB=${DIMS_UB})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/include")
file(GLOB_RECURSE pivot_lib_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM pivot_lib_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_library(pivot ${pivot_lib_SRC})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  find_package(OpenMP REQUIRED)
  target_link_libraries(pivot PRIVATE OpenMP::OpenMP_CXX)
endif()

add_executable(pivot_exec "src/main.cpp")
set_property(TARGET pivot_exec PROPERTY OUTPUT_NAME pivot)
target_link_libraries(pivot_exec PUBLIC pivot PRIVATE CLI11::CLI11)

add_subdirectory(tests)
