cmake_minimum_required(VERSION 3.0.0)
project(pivot VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
include_directories("/usr/include/graphviz")

file(GLOB_RECURSE pivot_SRC "src/*.cpp")
add_executable(pivot ${pivot_SRC})
target_link_libraries(pivot ${Boost_LIBRARIES})
target_link_libraries(pivot "gvc" "cgraph")
