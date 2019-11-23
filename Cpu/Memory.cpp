
#include <iostream>
#include "Cpu/Memory.hpp"
#include "Devices/Io.hpp"
#include "Util/LogUtil.hpp"

using namespace std;

Memory::Memory(uint8_t *bootRom, uint8_t* gameRom, Mbc *memoryBankController, PersistentRAM *persistentRAM) {
    this->bootRom = bootRom;
    this->gameRom = gameRom;
    this->memoryBankController = memoryBankController;
    this->persistentRAM = persistentRAM;
    memory = new uint8_t[MEMORY_SIZE];
}

Memory::~Memory() {
    delete[] memory;
    delete[] bootRom;
    delete[] gameRom;
    delete memoryBankController;
    delete persistentRAM;
}

void Memory::registerIoDevice(uint16_t address, IoDevice *ioDevice) {
    ioMap[address] = ioDevice;
}

uint16_t Memory::read16(uint16_t address) {
    uint32_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	return (uint16_t)decodedMemory[decodedAddress] | ((uint16_t)decodedMemory[decodedAddress+1] << 8);
}

uint8_t Memory::read8(uint16_t address) {
    auto ioMapping = ioMap.find(address);
    if (ioMapping != ioMap.end()) {
        return ioMap[address]->read8(address);
    }

    uint32_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);

	return decodedMemory[decodedAddress];
}

void Memory::write16(uint16_t address, uint16_t value) {
    uint32_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	decodedMemory[decodedAddress] = (uint8_t)(value & 0xFF);
	decodedMemory[decodedAddress+1] = (uint8_t)((value & 0xFF00) >> 8);
}

void Memory::write8(uint16_t address, uint8_t value) {
    auto ioMapping = ioMap.find(address);
    if (ioMapping != ioMap.end()) {
        ioMap[address]->write8(address, value);
        return;
    }

    if (memoryBankController->write8(address, value)) {
        return;
    }

    if (address < VIDEO_RAM_START) {
        cout << "Writing to rom!" << endl;
    }

    uint32_t decodedAddress = address;
	uint8_t *decodedMemory = getMemoryAreaForAddress(&decodedAddress);
	decodedMemory[decodedAddress] = value;


    // if (address == 0xac04) {
    //     cout << "Wrote: " << cout16Hex(value) << " into 0xac04" << endl;
    // }
}

uint8_t * Memory::getMemoryAreaForAddress(uint32_t *address) {
    if (*address < BOOT_ROM_SIZE && bootRomEnabled()) {
        return bootRom;

    } else if (*address >= ROM_BANK_SWTC_START && *address < (ROM_BANK_SWTC_START + ROM_BANK_SWTC_SIZE)) {
       // cout << "Accessing bank " << cout8Hex(memoryBankController->getRomBankNumber()) << endl;
        *address = (*address-ROM_BANK_SWTC_START) + (ROM_BANK_SIZE * memoryBankController->getRomBankNumber());
       // cout << "absolute address: " << cout16Hex(*address) << endl;
        return gameRom;

    } else if (*address < VIDEO_RAM_START) {
        return gameRom;

    } else if (*address >= RAM_BANK_SWTC_START && *address < (RAM_BANK_SWTC_START + RAM_BANK_SWTC_SIZE)) {
        *address -= RAM_BANK_SWTC_START;
        return persistentRAM->getRamBank(memoryBankController->getRamBankNumber());

    } else if (*address >= INTERNAL_RAM_ECHO_START && *address < OAM_RAM_START) {
        *address = INTERNAL_RAM_START + (*address-INTERNAL_RAM_ECHO_START);
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
