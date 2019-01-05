
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

void Memory::registerIoDevice(uint16_t address, IoDevice *ioDevice) {
    ioMap[address] = ioDevice;
}

uint16_t Memory::read16(uint16_t address, bool trace) {
    traceEnabled = trace;
    reading = true;
    uint16_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	return (uint16_t)decodedMemory[decodedAddress] | ((uint16_t)decodedMemory[decodedAddress+1] << 8);
}

uint8_t Memory::read8(uint16_t address, bool trace) {
    auto ioMapping = ioMap.find(address);
    if (ioMapping != ioMap.end()) {
        return ioMap[address]->read8(address);
    }

    traceEnabled = trace;
    reading = true;
    uint16_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	return decodedMemory[decodedAddress];
}

void Memory::write16(uint16_t address, uint16_t value, bool trace) {
    traceEnabled = trace;
    reading = false;
    uint16_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	decodedMemory[decodedAddress] = (uint8_t)(value & 0xFF);
	decodedMemory[decodedAddress+1] = (uint8_t)((value & 0xFF00) >> 8);
}

void Memory::write8(uint16_t address, uint8_t value, bool trace) {
    auto ioMapping = ioMap.find(address);
    if (ioMapping != ioMap.end()) {
        ioMap[address]->write8(value);
    }

    traceEnabled = trace;
    reading = false;
    uint16_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	decodedMemory[decodedAddress] = value;
}

uint8_t * Memory::getMemoryAreaForAddress(uint16_t *address) {

    if (traceEnabled && 
        *address >= IO_START && 
        *address < (IO_START+IO_MAPPED_SIZE) && 
        *address == DMA) {
        TRACE_IO("Access to IO " << cout16Hex(*address) << " for " << (reading ? "reading" : "writing") << endl);
    }

    if (*address < BOOT_ROM_SIZE && bootRomEnabled()) {
        return bootRom;
    } else if (*address < VIDEO_RAM_START) {
        return gameRom;
    } else if (*address >= INTERNAL_RAM_ECHO_START && *address < OAM_RAM_START) {
        *address -= INTERNAL_RAM_SIZE;
        return memory;
    } else {
        return memory;
    }
}

bool Memory::bootRomEnabled() {
    return read8(BOOT_ROM_DISABLE_REG) == 0;
}
