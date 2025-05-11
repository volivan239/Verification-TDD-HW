cmake_minimum_required(VERSION 3.21)
project(kv_storage CXX)

set(CMAKE_CXX_FLAGS -Wall -Wextra --coverage)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

add_executable(storage_test tests/test.cpp)
target_include_directories(storage_test PUBLIC src)
target_link_libraries(storage_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(storage_test)
