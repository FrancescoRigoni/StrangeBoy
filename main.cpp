#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>

#include "Io.hpp"
#include "Memory.hpp"
#include "PPU.hpp"
#include "Cpu.hpp"
#include "Joypad.hpp"
#include "Dma.hpp"
#include "LCDControlAndStat.hpp"
#include "InterruptFlags.hpp"
#include "LogUtil.hpp"

using namespace std;

uint8_t *readRom(const char *fileName);

int main(int argc, char **argv) {
	cout << "Hello Gameboy" << endl;

    uint8_t *bootRom = readRom("bootrom.bin");
    uint8_t *tetris = readRom("tetris.bin");

    Memory memory(bootRom, tetris);

    Joypad joypad;
    Dma dma(&memory);
    LCDControlAndStat lcdControlAndStat;
    InterruptFlags interruptFlags;

    memory.registerIoDevice(P1, &joypad);
    memory.registerIoDevice(DMA, &dma);
    memory.registerIoDevice(LCDC, &lcdControlAndStat);
    memory.registerIoDevice(STAT, &lcdControlAndStat);
    memory.registerIoDevice(SCY, &lcdControlAndStat);
    memory.registerIoDevice(SCX, &lcdControlAndStat);
    memory.registerIoDevice(LY, &lcdControlAndStat);
    memory.registerIoDevice(LYC, &lcdControlAndStat);
    memory.registerIoDevice(IF, &interruptFlags);
    memory.registerIoDevice(INTERRUPTS_ENABLE_REG, &interruptFlags);

    PPU ppu(&memory, &lcdControlAndStat, &interruptFlags);
    Cpu cpu(&memory, &interruptFlags);

    double ppuUpdateFrequencyHz = 60.0;
    int totalNumerOfRows = 154;
    double rowDrawFrequencyHz = ppuUpdateFrequencyHz * totalNumerOfRows;
    double mainLoopPeriodUs = 1;//(1000.0 / rowDrawFrequencyHz)*1000;
    //double mainLoopPeriodUs = 1000000;
    int lineCounter = 0;
    do {
        //ppu.doOAMSearch();
        //ppu.doHBlank();

        //for (int i = 0; i < (456/8); i++) {
            //dma.cycle(8);
            cpu.cycle(ppu.run());
            ppu.nextState();
        //}


        this_thread::sleep_for(chrono::microseconds((int)mainLoopPeriodUs));

        lineCounter++;
        lineCounter %= totalNumerOfRows;

    } while (!cpu.unimplemented);
    cout << "End" << endl;
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



