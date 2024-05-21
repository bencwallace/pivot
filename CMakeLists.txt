cmake_minimum_required(VERSION 3.0.0)
project(pivot VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic")
set(DEBUG_FLAGS "-O0 -g -D_GLIBCXX_DEBUG")
set(SANITIZE_FLAGS "-fsanitize=address,leak,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZE_FLAGS}")
elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
  message(WARNING "Compiling without clang. Disabling sanitizers.")
endif()

include(FetchContent)
FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG ccd6801
)
FetchContent_MakeAvailable(cli11_proj)

find_package(Boost)

file(GLOB_RECURSE pivot_lib_SRC "src/utils.cpp" "src/walk.cpp" "src/walk_tree.cpp")
add_library(pivot ${pivot_lib_SRC})
target_include_directories(pivot PUBLIC "/usr/include/graphviz" PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(pivot PRIVATE "gvc" "cgraph")

add_executable(pivot_exec "src/main.cpp")
set_property(TARGET pivot_exec PROPERTY OUTPUT_NAME pivot)
target_link_libraries(pivot_exec PUBLIC pivot PRIVATE CLI11::CLI11)

add_subdirectory(tests)
