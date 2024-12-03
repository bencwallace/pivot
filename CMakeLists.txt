cmake_minimum_required(VERSION 3.19.0)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
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

find_package(CLI11 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS headers preprocessor unordered)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GVC REQUIRED libgvc)
pkg_check_modules(CGRAPH REQUIRED libcgraph)

option(DIMS_UB "Upper bound on dimensions handled" OFF)
if(NOT DIMS_UB)
  set(DIMS_UB 6 CACHE STRING "Default upper bound on dimensions handled" FORCE)
endif()
add_definitions(-DDIMS_UB=${DIMS_UB})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/include")
file(GLOB_RECURSE pivot_lib_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM pivot_lib_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(pivot ${pivot_lib_SRC})
target_link_libraries(pivot PUBLIC Boost::headers)
target_include_directories(pivot PRIVATE gvc)

target_include_directories(pivot PUBLIC ${GVC_INCLUDE_DIRS} ${CGRAPH_INCLUDE_DIRS})
target_link_libraries(pivot PUBLIC ${GVC_LIBRARIES} ${CGRAPH_LIBRARIES})

add_executable(pivot_exec "src/main.cpp")
set_property(TARGET pivot_exec PROPERTY OUTPUT_NAME pivot)
target_link_libraries(pivot_exec PUBLIC pivot PRIVATE CLI11::CLI11)

add_subdirectory(tests)
