cmake_minimum_required (VERSION 2.6)
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

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

# All warnings on
set (CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

set (CMAKE_CXX_FLAGS "-I/usr/local/include -I. ${CMAKE_CXX_FLAGS}")

file(GLOB sources *.cpp 
                  Cpu/*.cpp 
                  Memory/*.cpp 
                  Devices/*.cpp 
                  Util/*.cpp 
                  PPU/*.cpp 
                  UI/*.cpp
                  MBC/*.cpp)

add_executable(StrangeBoy ${sources})
target_link_libraries(StrangeBoy ${SDL2_LIBRARY})

