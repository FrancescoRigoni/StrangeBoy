cmake_minimum_required (VERSION 3.9)
project(StrangeBoy VERSION 1.0.0 DESCRIPTION "Gameboy emulator library")

if (CMAKE_VERSION VERSION_LESS "3.1")
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set (CMAKE_CXX_FLAGS "--std=gnu++14 -lpthread ${CMAKE_CXX_FLAGS}")
endif ()
else ()
    set (CMAKE_CXX_STANDARD 14)
endif ()

# All warnings on
set (CMAKE_CXX_FLAGS "-Wall ${CMAKE_CXX_FLAGS}")

set (CMAKE_CXX_FLAGS "-I/usr/local/include ${CMAKE_CXX_FLAGS}")

add_library(StrangeBoy SHARED 
./Cartridge.cpp
./Cpu/Memory.cpp
./Cpu/PersistentRAM.cpp
./Cpu/Cpu.cpp
./MBC/Mbc1.cpp
./PPU/PPU.cpp
./Devices/SoundChannelWave.cpp
./Devices/Dma.cpp
./Devices/Timer.cpp
./Devices/SoundChannelSquareWave.cpp
./Devices/SoundChannelNoise.cpp
./Devices/InterruptFlags.cpp
./Devices/Audio/EnvelopeCounter.cpp
./Devices/Audio/LengthCounter.cpp
./Devices/Audio/FrequencyCounter.cpp
./Devices/Audio/SweepCounter.cpp
./Devices/Joypad.cpp
./Devices/Serial.cpp
./Devices/DivReg.cpp
./Devices/APU.cpp
./Devices/LCDRegs.cpp)

target_include_directories (StrangeBoy PUBLIC
.
./Cpu/
./MBC/
./PPU/
./Devices/)

