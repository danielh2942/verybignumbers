cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)

project(test VERSION 1.0 LANGUAGES CXX)


add_executable(test test.cpp)

add_executable(test_readable test_readable.cpp)

include(cmake/CPM.cmake)
CPMAddPackage("gh:catchorg/Catch2@3.5.2")

target_link_libraries(test PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test_readable PRIVATE Catch2::Catch2WithMain)
