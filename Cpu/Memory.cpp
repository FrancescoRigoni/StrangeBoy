
#include <iostream>
#include "Cpu/Memory.hpp"
#include "Devices/Io.hpp"
#include "Util/LogUtil.hpp"

using namespace std;

Memory::Memory(uint8_t *bootRom, uint8_t* gameRom, Mbc *memoryBankController) {
    this->bootRom = bootRom;
    this->gameRom = gameRom;
    this->memoryBankController = memoryBankController;
    memory = new uint8_t[MEMORY_SIZE];
}

Memory::~Memory() {
    delete[] memory;
    delete[] bootRom;
    delete[] gameRom;
    delete memoryBankController;
}

void Memory::registerIoDevice(uint16_t address, IoDevice *ioDevice) {
    ioMap[address] = ioDevice;
}

uint16_t Memory::read16(uint16_t address, bool trace) {
    uint16_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	return (uint16_t)decodedMemory[decodedAddress] | ((uint16_t)decodedMemory[decodedAddress+1] << 8);
}

uint8_t Memory::read8(uint16_t address, bool trace) {
    auto ioMapping = ioMap.find(address);
    if (ioMapping != ioMap.end()) {
        return ioMap[address]->read8(address);
    }

    uint16_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	return decodedMemory[decodedAddress];
}

void Memory::write16(uint16_t address, uint16_t value, bool trace) {
    uint16_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	decodedMemory[decodedAddress] = (uint8_t)(value & 0xFF);
	decodedMemory[decodedAddress+1] = (uint8_t)((value & 0xFF00) >> 8);
}

void Memory::write8(uint16_t address, uint8_t value, bool trace) {
    auto ioMapping = ioMap.find(address);
    if (ioMapping != ioMap.end()) {
        ioMap[address]->write8(address, value);
        return;
    }

    if (memoryBankController != 0) {
        if (memoryBankController->write8(address, value)) {
            return;
        }
    }

    uint16_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	decodedMemory[decodedAddress] = value;
}

uint8_t * Memory::getMemoryAreaForAddress(uint16_t *address) {
    if (*address < BOOT_ROM_SIZE && bootRomEnabled()) {
        return bootRom;
    } else if (memoryBankController != 0 &&
               *address >= ROM_BANK_SWTC_START && *address < (ROM_BANK_SWTC_START + ROM_BANK_SWTC_SIZE)) {
        *address = (*address-ROM_BANK_SWTC_START) + (ROM_BANK_SIZE * memoryBankController->getRomBankNumber());
        return gameRom;
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

void * Memory::getRawPointer(uint16_t address) {
    return (void *)(memory + address);
}
