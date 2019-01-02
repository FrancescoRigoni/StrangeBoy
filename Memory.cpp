
#include <iostream>
#include "Memory.hpp"
#include "Io.hpp"
#include "LogUtil.hpp"
using namespace std;

Memory::Memory(uint8_t *bootRom, uint8_t* gameRom) {
    this->bootRom = bootRom;
    this->gameRom = gameRom;
    memory = new uint8_t[MEMORY_SIZE];
}

Memory::~Memory() {
    delete[] memory;
    delete[] bootRom;
    delete[] gameRom;
}

uint16_t Memory::read16(uint16_t address) {
    reading = true;
	uint8_t *decodedMemory = getMemoryAreaForAddress(address);
	return (uint16_t)decodedMemory[address] | ((uint16_t)decodedMemory[address+1] << 8);
}

uint8_t Memory::read8(uint16_t address) {
    reading = true;
	uint8_t *decodedMemory = getMemoryAreaForAddress(address);
	return decodedMemory[address];
}

void Memory::write16(uint16_t address, uint16_t value) {
    reading = false;
	uint8_t *decodedMemory = getMemoryAreaForAddress(address);
	decodedMemory[address] = (uint8_t)(value & 0xFF);
	decodedMemory[address+1] = (uint8_t)((value & 0xFF00) >> 8);
}

void Memory::write8(uint16_t address, uint8_t value) {
    reading = false;
	uint8_t *decodedMemory = getMemoryAreaForAddress(address);
	decodedMemory[address] = value;
}

uint8_t * Memory::getMemoryAreaForAddress(uint16_t address) {
    if (address < BOOT_ROM_SIZE && bootRomEnabled()) {
        return bootRom;
    } else if (address < VIDEO_RAM_START) {
        return gameRom;
    } else {
        return memory;
    }
}

bool Memory::bootRomEnabled() {
    return read8(BOOT_ROM_DISABLE_REG) == 0;
}
