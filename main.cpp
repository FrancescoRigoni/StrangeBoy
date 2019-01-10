#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>
#include <atomic>

#include "Io.hpp"
#include "Memory.hpp"
#include "PPU.hpp"
#include "Cpu.hpp"
#include "Joypad.hpp"
#include "Dma.hpp"
#include "LCDRegs.hpp"
#include "InterruptFlags.hpp"
#include "LogUtil.hpp"

using namespace std;

uint8_t *readRom(const char *);
void runGameBoy(Screen *, atomic<bool> *);

int main(int argc, char **argv) {
    atomic<bool> exit(false);

    Screen screen;
    thread gameboyThread(runGameBoy, &screen, &exit);
    screen.run();

    exit = true;
    gameboyThread.join();
}

void runGameBoy(Screen *screen, atomic<bool> *exit) {
    uint8_t *bootRom = readRom("bootrom.bin");
    uint8_t *tetris = readRom("tetris.bin");

    Memory memory(bootRom, tetris);
    uint8_t cartType = memory.read16(0x0147);

    if (cartType != 0) {
        cout << "Cartridge type is " << cout8Hex(cartType) << ". Still not handled" << endl;
        return;
    }

    Joypad joypad;
    Dma dma(&memory);
    LCDRegs lcdRegs;
    InterruptFlags interruptFlags;

    memory.registerIoDevice(P1, &joypad);
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

    double ppuUpdateFrequencyHz = 60.0;
    int totalNumerOfRows = 154;
    double rowDrawFrequencyHz = ppuUpdateFrequencyHz * totalNumerOfRows;
    double mainLoopPeriodUs = (1000.0 / rowDrawFrequencyHz)*1000;

    double cpuClockSpeedMhz = 4.194304;
    double oneCyclePeriodUs = 1 / cpuClockSpeedMhz;

    do {
        //for (int i = 0; i < (456/8); i++) {
            int cycles = ppu.run();
            dma.cycle(cycles);
            cpu.cycle(cycles);
            ppu.nextState();
        ///}

        //double toSleepUs = oneCyclePeriodUs * cycles;
        //this_thread::sleep_for(chrono::microseconds((int)toSleepUs));

    } while (!cpu.unimplemented && !exit->load());
}

uint8_t *readRom(const char *fileName) {
    ifstream rom;
    rom.open(fileName, ios::in | ios::binary);

    if (!rom.is_open()) {
        cout << "Unable to open file" << endl;
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



