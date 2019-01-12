#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>
#include <atomic>
#include <algorithm>

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
using namespace std::chrono;

uint8_t *readRom(const char *);
void runGameBoy(Screen *, Joypad *, atomic<bool> *);

int main(int argc, char **argv) {
    atomic<bool> exit(false);

    Joypad joypad;
    Screen screen(&joypad);
    thread gameboyThread(runGameBoy, &screen, &joypad, &exit);
    screen.run();

    exit = true;
    gameboyThread.join();
}

void runGameBoy(Screen *screen, Joypad *joypad, atomic<bool> *exit) {
    uint8_t *bootRom = readRom("bootrom.bin");
    uint8_t *tetris = readRom("tetris.bin");

    Memory memory(bootRom, tetris);
    uint8_t cartType = memory.read16(0x0147);

    if (cartType != 0) {
        cout << "Cartridge type is " << cout8Hex(cartType) << ". Still not handled" << endl;
        return;
    }

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

    int fpsFrequency = 240;
    int msRefreshPeriod = 1000 / fpsFrequency;

    do {
        // Draw a frame
        milliseconds msAtFrameStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

        do {
            int cycles = ppu.run();
            dma.cycle(cycles);
            cpu.cycle(cycles);
            ppu.nextState();
        } while (lcdRegs.read8(LY) != 0);

        milliseconds msAtFrameEnd = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
        milliseconds msTakenByDrawingFrame = msAtFrameEnd - msAtFrameStart;
        int msToSleep = max(0, msRefreshPeriod - (int)msTakenByDrawingFrame.count());
        //cout << "Frame took " << msTakenByDrawingFrame.count() << " ms, sleeping for " << msToSleep << " ms" << endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(msToSleep));

    } while (!cpu.unimplemented && !exit->load());

    cout << "GB thread terminating" << endl;

    return;
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



