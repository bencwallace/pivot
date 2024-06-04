cmake_minimum_required(VERSION 3.0.0)
project(pivot VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic" CACHE STRING "Flags used by the compiler during all builds." FORCE)

set(DEBUG_FLAGS "-O0 -g -D_GLIBCXX_DEBUG")
set(SANITIZE_FLAGS "-fsanitize=address,leak,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_definitions(-Wno-deprecated-declarations)  # TODO: re-enable this
  if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14)
    message(WARNING "clang-13 or less is recommended.")
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "${DEBUG_FLAGS} ${SANITIZE_FLAGS}" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
  message(WARNING "Compiling without clang. Disabling sanitizers.")
  set(CMAKE_CXX_FLAGS_DEBUG "${DEBUG_FLAGS}" CACHE STRING "Flags used by the compiler during debug builds." FORCE)
endif()

set(CMAKE_CXX_FLAGS_PROFILE "-O3 -g -fno-omit-frame-pointer" CACHE STRING "Flags used by the compiler during profile builds." FORCE)
set(CMAKE_BUILD TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "Build types: None Debug Release MinSizeRel RelWithDebInfo Profile" FORCE)

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
include_directories("/usr/include/graphviz")

file(GLOB_RECURSE pivot_lib_SRC "src/graphviz.cpp" "src/utils.cpp" "src/walk.h" "src/walk_tree.h")
add_library(pivot ${pivot_lib_SRC})

add_executable(pivot_exec "src/main.cpp")
set_property(TARGET pivot_exec PROPERTY OUTPUT_NAME pivot)
target_link_libraries(pivot_exec PUBLIC pivot PRIVATE CLI11::CLI11)

add_subdirectory(tests)
