cmake_minimum_required (VERSION 3.9)
project(mylib VERSION 1.0.0 DESCRIPTION "Gameboy emulator library")
project (StrangeBoy)

if (CMAKE_VERSION VERSION_LESS "3.1")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set (CMAKE_CXX_FLAGS "--std=gnu++14 -lpthread ${CMAKE_CXX_FLAGS}")
endif ()
else ()
    set (CMAKE_CXX_STANDARD 14)
endif ()

# includes cmake/FindSDL2.cmake
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# All warnings on
set (CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

set (CMAKE_CXX_FLAGS "-I/usr/local/include -I. ${CMAKE_CXX_FLAGS}")

file(GLOB sources *.cpp 
                  Cpu/*.cpp 
                  Memory/*.cpp 
                  Devices/*.cpp 
                  Devices/Audio/*.cpp 
                  Util/*.cpp 
                  PPU/*.cpp 
                  UI/*.cpp
                  MBC/*.cpp)

add_library(StrangeBoy SHARED ${sources})

