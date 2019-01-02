#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>

#include "Memory.hpp"
#include "Display.hpp"
#include "Cpu.hpp"
#include "LogUtil.hpp"

using namespace std;

uint8_t *readRom(const char *fileName);

int main(int argc, char **argv) {
	cout << "Hello Gameboy" << endl;

    uint8_t *bootRom = readRom("bootrom.bin");
    uint8_t *tetris = readRom("tetris.bin");

    Memory memory(bootRom, tetris);

    Display display(&memory);
    Cpu cpu(&memory);

    double displayUpdateFrequencyHz = 60.0;
    int totalNumerOfRows = 154;
    double rowDrawFrequencyHz = displayUpdateFrequencyHz * totalNumerOfRows;
    double mainLoopPeriodUs = (1000.0 / rowDrawFrequencyHz)*1000;

    int lineCounter = 0;
    do {
        //display.doOAMSearch();
        //display.doHBlank();

        // if (lineCounter == 0) {
        //     cout << "SCY: " << cout8Hex(memory.read8(SCY)) << endl;
        // }

        display.drawLine();
        cpu.cycle(456);

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



