#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>

#include "Io.hpp"
#include "Memory.hpp"
#include "PPU.hpp"
#include "Cpu.hpp"
#include "Joypad.hpp"
#include "LogUtil.hpp"

using namespace std;

uint8_t *readRom(const char *fileName);

int main(int argc, char **argv) {
	cout << "Hello Gameboy" << endl;

    uint8_t *bootRom = readRom("bootrom.bin");
    uint8_t *tetris = readRom("tetris.bin");

    Memory memory(bootRom, tetris);

    Joypad joypad;
    memory.registerIoDevice(P1, &joypad);
    PPU ppu(&memory);
    Cpu cpu(&memory);

    double ppuUpdateFrequencyHz = 60.0;
    int totalNumerOfRows = 154;
    double rowDrawFrequencyHz = ppuUpdateFrequencyHz * totalNumerOfRows;
    double mainLoopPeriodUs = (1000.0 / rowDrawFrequencyHz)*1000;
    //double mainLoopPeriodUs = 1000000;
    int lineCounter = 0;
    do {
        //ppu.doOAMSearch();
        //ppu.doHBlank();

        // if (lineCounter == 0) {
        //     cout << "SCY: " << cout8Hex(memory.read8(SCY)) << endl;
        // }

        cpu.cycle(456);
        ppu.drawLine();

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



