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
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(conesim
    src/main.cpp
)

target_include_directories(conesim
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)