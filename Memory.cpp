
#include <iostream>
#include "Memory.hpp"
#include "Io.hpp"
#include "LogUtil.hpp"
using namespace std;

Memory::Memory(uint8_t *bootRom, uint8_t* gameRom) {
    this->bootRom = bootRom;
    this->romBank0 = gameRom;
    this->romBankSwitchable = (gameRom+ROM_BANK_0_SIZE);

	highRam = new uint8_t[HIGH_RAM_SIZE];
	ioMapped = new uint8_t[IO_MAPPED_SIZE];
	ramOAM = new uint8_t[OAM_RAM_SIZE];
	internalRam = new uint8_t[INTERNAL_RAM_SIZE];
	ramBankSwitchable = new uint8_t[RAM_BANK_SWTC_SIZE];
	videoRam = new uint8_t[VIDEO_RAM_SIZE];
}

Memory::~Memory() {
	delete[] highRam;
	delete[] ioMapped;
	delete[] ramOAM;
	delete[] internalRam;
	delete[] ramBankSwitchable;
	delete[] videoRam;
	delete[] romBank0;
    delete[] bootRom;
}

#define DECODE_ADDR_AND_MEMORY(address) \
	uint16_t decodedAddress = address; \
	uint8_t *decodedMemory = decodeAddress(&decodedAddress); \
	
uint16_t Memory::read16(uint16_t address) {
    reading = true;
	DECODE_ADDR_AND_MEMORY(address);
	return (uint16_t)decodedMemory[decodedAddress] | ((uint16_t)decodedMemory[decodedAddress+1] << 8);
}

uint8_t Memory::read8(uint16_t address) {
    reading = true;
	DECODE_ADDR_AND_MEMORY(address);
	return decodedMemory[decodedAddress];
}

void Memory::write16(uint16_t address, uint16_t value) {
    reading = false;
	DECODE_ADDR_AND_MEMORY(address);
	decodedMemory[decodedAddress] = (uint8_t)(value & 0xFF);
	decodedMemory[decodedAddress+1] = (uint8_t)((value & 0xFF00) >> 8);
}

void Memory::write8(uint16_t address, uint8_t value) {
    reading = false;
	DECODE_ADDR_AND_MEMORY(address);
    //cout << endl << "Writing val " << cout8Hex(value) << endl;
	decodedMemory[decodedAddress] = value;
}

uint8_t * Memory::decodeAddress(uint16_t *address) {
	if (*address >= ROM_BANK_0_START && 
		*address < ROM_BANK_0_SIZE) {
        if (*address < 0x100 && read8(BOOT_ROM_DISABLE_REG) == 0) {
            return bootRom;
        } else {
            return romBank0;
        }
	} else if (*address >= ROM_BANK_SWTC_START && 
		*address < (ROM_BANK_SWTC_START + ROM_BANK_SWTC_SIZE)) {
		*address -= ROM_BANK_SWTC_START;
		return romBankSwitchable;

	} else if (*address >= VIDEO_RAM_START && 
		*address < (VIDEO_RAM_START + VIDEO_RAM_SIZE)) {
		*address -= VIDEO_RAM_START;
        //if (!reading) TRACE_VRAM(endl << "Access to VRAM at address " << cout16Hex(*address + VIDEO_RAM_START) << endl);
		return videoRam;

	} else if (*address >= RAM_BANK_SWTC_START && 
		*address < (RAM_BANK_SWTC_START+RAM_BANK_SWTC_SIZE)) {
		*address -= RAM_BANK_SWTC_START;
		return ramBankSwitchable;

	} else if (*address >= INTERNAL_RAM_START && 
		       *address < (INTERNAL_RAM_START+INTERNAL_RAM_SIZE)) {
		*address -= INTERNAL_RAM_START;
		return internalRam;

	} else if (*address >= INTERNAL_RAM_ECHO_START && 
		       *address < (INTERNAL_RAM_ECHO_START+INTERNAL_RAM_ECHO_SIZE)) {
		*address -= INTERNAL_RAM_START;
		return internalRam;

	} else if (*address >= OAM_RAM_START && 
		*address < (OAM_RAM_START+OAM_RAM_SIZE)) {
		*address -= OAM_RAM_START;
		return ramOAM;

	} else if (*address >= IO_START && 
		*address < (IO_START+IO_MAPPED_SIZE)) {
		*address -= IO_START;
		return ioMapped;

	} else if (*address >= HIGH_RAM_START && 
		*address < (HIGH_RAM_START+HIGH_RAM_SIZE)) {
		*address -= HIGH_RAM_START;
		return highRam;

	} else if (*address == INTERRUPTS_ENABLE_REG) {
        *address = 0;
		return &interruptEnableReg;

	} else if (*address == BOOT_ROM_DISABLE_REG) {
        *address = 0;
        return &bootRomDisableReg;

    } else {
		*address = 0;
		return 0;
	}
}
