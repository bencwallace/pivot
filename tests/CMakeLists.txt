cmake_minimum_required(VERSION 3.19.0)
project(pivot_tests VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)

add_executable(test_pivot test_utils.h int_test.cpp lattice_test.cpp walk_node_test.cpp walk_tree_test.cpp)
target_include_directories(test_pivot PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_pivot pivot GTest::gtest_main)

gtest_discover_tests(test_pivot)
