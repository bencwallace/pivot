cmake_minimum_required(VERSION 3.0.0)
project(pivot VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic")
set(DEBUG_FLAGS "-O0 -g -D_GLIBCXX_DEBUG")
set(SANITIZE_FLAGS "-fsanitize=address,leak,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZE_FLAGS}")
elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
  message(WARNING "Compiling without clang. Disabling sanitizers.")
endif()

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories("/usr/include/graphviz")

file(GLOB_RECURSE pivot_lib_SRC "src/utils.cpp" "src/walk.cpp" "src/walk_tree.cpp")
add_library(pivot_lib ${pivot_lib_SRC})
add_executable(pivot "src/main.cpp")
target_link_libraries(pivot pivot_lib ${Boost_LIBRARIES})
target_link_libraries(pivot_lib ${Boost_LIBRARIES})
target_link_libraries(pivot_lib "gvc" "cgraph")

add_subdirectory(tests)
