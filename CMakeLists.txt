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

include_directories(include)

# ==========================================
# TARGET: conesim_core
# ==========================================
file(GLOB CORE_SOURCES src/core/*.cpp)
add_library(conesim_core ${CORE_SOURCES})
target_include_directories(conesim_core PUBLIC include)

# ==========================================
# TARGET: conesim_routing
# ==========================================
file(GLOB ROUTING_SOURCES src/routing/*.cpp src/routing/community/*.cpp)
add_library(conesim_routing ${ROUTING_SOURCES})
target_include_directories(conesim_routing PUBLIC include)

# ==========================================
# TARGET: conesim_movement
# ==========================================
file(GLOB MOVEMENT_SOURCES src/movement/*.cpp)
add_library(conesim_movement ${MOVEMENT_SOURCES})
target_include_directories(conesim_movement PUBLIC include)

# ==========================================
# TARGET: conesim_input
# ==========================================
file(GLOB INPUT_SOURCES src/input/*.cpp)
add_library(conesim_input ${INPUT_SOURCES})
target_include_directories(conesim_input PUBLIC include)

# ==========================================
# TARGET: conesim_report
# ==========================================
file(GLOB REPORT_SOURCES src/report/*.cpp)
add_library(conesim_report ${REPORT_SOURCES})
target_include_directories(conesim_report PUBLIC include)

# Connecting modules to the core, as they require the system core.
target_link_libraries(conesim_routing PUBLIC conesim_core)
target_link_libraries(conesim_movement PUBLIC conesim_core)
target_link_libraries(conesim_input PUBLIC conesim_core)
target_link_libraries(conesim_report PUBLIC conesim_core)

# ==========================================
# DEPENDENCY: GoogleTest
# ==========================================
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ==========================================
# TARGET: conesim
# ==========================================
add_executable(conesim src/main.cpp)
target_link_libraries(conesim
    PRIVATE
    conesim_routing
    conesim_movement
    conesim_input
    conesim_report
)

# ==========================================
# TARGET: run_tests (Unit Testing)
# ==========================================
enable_testing()

# Collect all test files from the core, routing and movement folders
file(GLOB TEST_SOURCES 
    tests/core/*.cpp 
    tests/routing/community/*.cpp 
    tests/movement/*.cpp
    tests/input/*.cpp
    tests/report/*.cpp
)

add_executable(run_tests ${TEST_SOURCES})
target_include_directories(run_tests PRIVATE include)

# Link routing and movement
target_link_libraries(run_tests
    PRIVATE
    conesim_routing
    conesim_movement
    conesim_input
    conesim_report
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)

# ==========================================
# DOKUMENTASI: Doxygen
# ==========================================
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