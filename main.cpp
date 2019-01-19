#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>
#include <atomic>
#include <algorithm>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "Devices/Io.hpp"
#include "Cpu/Memory.hpp"
#include "PPU/PPU.hpp"
#include "Cpu/Cpu.hpp"
#include "Devices/Joypad.hpp"
#include "Devices/Dma.hpp"
#include "Devices/LCDRegs.hpp"
#include "Devices/InterruptFlags.hpp"
#include "Util/LogUtil.hpp"
#include "Screen/Screen.hpp"
#include "Cartridge.hpp"

using namespace std;
using namespace std::chrono;

uint8_t *readRom(const char *);
void runGameBoy(const char *romPath, Screen *, Joypad *, atomic<bool> *);

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char **argv) {
    signal(SIGSEGV, handler);

    atomic<bool> exit(false);

    Joypad joypad;
    Screen screen(&joypad);
    thread gameboyThread(runGameBoy, argv[1], &screen, &joypad, &exit);
    screen.run();

    exit = true;
    gameboyThread.join();
}

unsigned long getTimeMilliseconds() {
    return chrono::duration_cast<chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
}

void runGameBoy(const char *romPath, Screen *screen, Joypad *joypad, atomic<bool> *exit) {
    uint8_t *bootRom = readRom("roms/bootrom.bin");
    uint8_t *gameRom = readRom(romPath);

    Cartridge cartridge;
    CartridgeInfo *cartridgeInfo = 0;
    cartridge.parse(gameRom, cartridgeInfo);

    Memory memory(bootRom, gameRom);
    Dma dma(&memory);
    LCDRegs lcdRegs;
    InterruptFlags interruptFlags;

    memory.registerIoDevice(P1, joypad);
    memory.registerIoDevice(DMA, &dma);
    memory.registerIoDevice(LCDC, &lcdRegs);
    memory.registerIoDevice(STAT, &lcdRegs);
    memory.registerIoDevice(SCY, &lcdRegs);
    memory.registerIoDevice(SCX, &lcdRegs);
    memory.registerIoDevice(LY, &lcdRegs);
    memory.registerIoDevice(LYC, &lcdRegs);
    memory.registerIoDevice(IF, &interruptFlags);
    memory.registerIoDevice(INTERRUPTS_ENABLE_REG, &interruptFlags);

    PPU ppu(&memory, &lcdRegs, &interruptFlags, screen);
    Cpu cpu(&memory, &interruptFlags);

    int fpsFrequency = 60*4;
    unsigned long msRefreshPeriod = 1000 / fpsFrequency;

    // Give some time to the screen window to display
    this_thread::sleep_for(chrono::milliseconds(1000));

    do {
        // Draw a frame
        unsigned long timeAtStartOfFrame = getTimeMilliseconds();

        do {
            int cycles = ppu.run();
            dma.cycle(cycles);
            cpu.cycle(cycles);
            ppu.nextState();
        } while (lcdRegs.read8(LY) != 0);

        unsigned long msSpentProcessingFrame = getTimeMilliseconds() - timeAtStartOfFrame;
        int msStillToWaitForNextFrame = msRefreshPeriod - msSpentProcessingFrame;
        int msToSleep = max(0, msStillToWaitForNextFrame);
        this_thread::sleep_for(chrono::milliseconds(msToSleep));

    } while (!cpu.unimplemented && !exit->load());

    return;
}

uint8_t *readRom(const char *fileName) {
    ifstream rom;
    rom.open(fileName, ios::in | ios::binary);

    if (!rom.is_open()) {
        cout << "Unable to open file " << fileName << endl;
        exit(1);
    }

    rom.seekg(0, ios::end);
    int romSize = rom.tellg();
    char *romContent = new char[romSize];
    rom.seekg(0, ios::beg);
    rom.read(romContent, romSize);
    rom.close();

    return (uint8_t*)romContent;
}



