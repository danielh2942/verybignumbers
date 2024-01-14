cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)

project(test VERSION 1.0 LANGUAGES CXX)

find_package(Catch2 3 REQUIRED)

add_executable(test test.cpp)

add_executable(test_readable test_readable.cpp)

target_link_libraries(test PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test_readable PRIVATE Catch2::Catch2WithMain)
