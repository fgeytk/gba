cmake_minimum_required(VERSION 3.15)
project(gb_emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/bus.cpp
    src/cpu.cpp
)

add_executable(gb_emulator ${SOURCES})
