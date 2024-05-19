cmake_minimum_required(VERSION 3.0.0)
project(pivot_tests VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

include(GoogleTest)
include(CTest)

add_executable(test_pivot int_test.cpp)
target_include_directories(test_pivot PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(test_pivot pivot GTest::gtest_main)

gtest_discover_tests(test_pivot)
