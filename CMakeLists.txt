#[[
    Project Configuration,
    Created by Neville on September 2026
]]

cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0135 NEW)

project(conesim
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_library(conesim_core
    src/core/Configuration.cpp
    src/core/SimulationClock.cpp
    src/core/Debug.cpp
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
    tests/core/test_Configuration.cpp
    tests/core/test_SimulationClock.cpp
    tests/core/test_Debug.cpp
)

target_link_libraries(run_tests
    conesim_core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)

find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_PROJECT_BRIEF "Opportunistic Network Simulation Engine")
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs")
    
    set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    
    set(DOXYGEN_HAVE_DOT YES)
    set(DOXYGEN_CLASS_GRAPH YES)
    set(DOXYGEN_CALL_GRAPH YES)
    set(DOXYGEN_CALLER_GRAPH YES)
    
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_LATEX NO)

    doxygen_add_docs(
        docs
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        COMMENT "Generating API documentation with Doxygen"
    )
else()
    message(WARNING "Doxygen not found, documentation target 'docs' will not be available.")
endif()