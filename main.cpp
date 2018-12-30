#include <iostream>
#include <fstream>

#include "Memory.hpp"
#include "Cpu.hpp"

using namespace std;

int main(int argc, char **argv) {
	cout << "Hello Gameboy" << endl;

	ifstream bootRom;
	bootRom.open("bootrom.bin", ios::in | ios::binary);

	if (!bootRom.is_open()) {
		cout << "Unable to open file" << endl;
		exit(1);
	}

    bootRom.seekg(0, ios::end);
	int bootRomSize = bootRom.tellg();
	char *bootRomContent = new char[bootRomSize];
	bootRom.seekg(0, ios::beg);
	bootRom.read(bootRomContent, bootRomSize);
	bootRom.close();

    Memory memory;

    cout << "Rom size " << bootRomSize << endl;
    for (int i = 0; i < bootRomSize; i++) {
        memory.write8(i, (uint8_t) bootRomContent[i]);
    }

    Cpu cpu(&memory);
    while (cpu.regPC != 0x0024 ) {
        cpu.cycle();
    }
    cout << "End" << endl;

	delete[] bootRomContent;
}