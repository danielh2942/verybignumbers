cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 20)

project(VeryBigNums VERSION 1.0 LANGUAGES CXX)

add_executable(test_poc test.cpp)

add_executable(test_readable test_readable.cpp)

add_executable(test_coldstorage_uuid test_coldstorage_uuid.cpp)

add_executable(test_coldvector test_coldvector.cpp)

add_library(ColdStorage STATIC coldstorage.cpp)

add_library(MyUtils STATIC util.cpp)

include(cmake/CPM.cmake)
CPMAddPackage("gh:catchorg/Catch2@3.5.2")
CPMAddPackage("gh:crashoz/uuid_v4#bae4100")

# ColdStorage needs avx2 support because of UUID stuff
# TODO:Add check and error-out if not supported
target_compile_options(ColdStorage PUBLIC -mavx -mavx2)
target_compile_options(MyUtils PUBLIC -mavx -mavx2)

target_link_libraries(test_poc PRIVATE Catch2::Catch2WithMain)
target_link_libraries(test_readable PRIVATE Catch2::Catch2WithMain)
target_link_libraries(ColdStorage PRIVATE uuid_v4::uuid_v4)
target_link_libraries(MyUtils PRIVATE uuid_v4::uuid_v4)
target_link_libraries(test_coldstorage_uuid PRIVATE ColdStorage)
target_link_libraries(test_coldvector PRIVATE Catch2::Catch2WithMain MyUtils)

include(CTest)

enable_testing()
add_test(NAME VeryBigNums.ProofOfConcept COMMAND test_poc)
add_test(NAME VeryBigNums.Readable COMMAND test_readable)
add_test(NAME VeryBigNums.ColdVector COMMAND test_coldvector)

