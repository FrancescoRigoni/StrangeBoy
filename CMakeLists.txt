cmake_minimum_required (VERSION 3.9)
project (StrangeBoyEmu)

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

add_executable(StrangeBoyEmu 
./UI/UI.cpp
./UI/StupidSound.cpp
./UI/StupidScreen.cpp
./main.cpp)

add_subdirectory(./lib/)
add_dependencies(StrangeBoyEmu StrangeBoy)
target_link_libraries(StrangeBoyEmu ${SDL2_LIBRARY} StrangeBoy)

