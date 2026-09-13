#[[
    Project Configuration,
    Created by Neville on September 2026
]]

cmake_minimum_required(VERSION 3.20)

project(conesim
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_library(conesim_core
    src/core/Configuration.cpp
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(run_tests
    tests/test_Configuration.cpp
)

target_link_libraries(run_tests
    conesim_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)