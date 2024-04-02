cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)

project(VeryBigNums VERSION 1.0 LANGUAGES CXX)

add_executable(test_poc test.cpp)

add_executable(test_readable test_readable.cpp)

add_executable(test_coldstorage_uuid test_coldstorage_uuid.cpp)

add_executable(test_coldvector test_coldvector.cpp)

add_executable(test_fixed_bignum test_fixed_bignum.cpp)

add_executable(test_arbitrary_bignum test_arbitrary_bignum.cpp)

add_executable(demo demo.cpp)

add_executable(factorialtest main.cpp)

add_library(ColdStorage STATIC coldstorage.cpp)

add_library(MyUtils STATIC util.cpp)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
# set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")

include(cmake/CPM.cmake)
CPMAddPackage("gh:catchorg/Catch2@3.5.2")
CPMAddPackage("gh:crashoz/uuid_v4#bae4100")
#CPMAddPackage("gh:ned14/quickcpplib#72277c7")
#CPMAddPackage("gh:ned14/llfio#418a2e9")

# ColdStorage needs avx2 support because of UUID stuff
# TODO:Add check and error-out if not supported
target_compile_options(ColdStorage PUBLIC -mavx -mavx2)
target_compile_options(MyUtils PUBLIC -mavx -mavx2)

target_link_libraries(test_poc PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test_readable PRIVATE Catch2::Catch2WithMain MyUtils)
target_link_libraries(ColdStorage PRIVATE uuid_v4::uuid_v4)
target_link_libraries(MyUtils PRIVATE uuid_v4::uuid_v4)
target_link_libraries(test_coldstorage_uuid PRIVATE ColdStorage)
target_link_libraries(test_coldvector PRIVATE Catch2::Catch2WithMain MyUtils)
target_link_libraries(test_fixed_bignum PRIVATE Catch2::Catch2WithMain MyUtils)
target_link_libraries(test_arbitrary_bignum PRIVATE Catch2::Catch2WithMain MyUtils)
target_link_libraries(factorialtest PRIVATE MyUtils)
target_link_libraries(demo PRIVATE MyUtils)

include(CTest)
include(cmake/Catch.cmake)

catch_discover_tests(test_poc)
catch_discover_tests(test_readable)
catch_discover_tests(test_coldvector)
catch_discover_tests(test_fixed_bignum)
catch_discover_tests(test_arbitrary_bignum)

#add_subdirectory(experiment)
