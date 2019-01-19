
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstdint>
#include <unordered_map> 
#include "Devices/IoDevice.hpp"

using namespace std;

#define MEMORY_SIZE                 0x10000

#define BOOT_ROM_DISABLE_REG        0xFF50
#define HIGH_RAM_START              0xFF80
#define IO_START                    0xFF00
#define OAM_RAM_START               0xFE00
#define INTERNAL_RAM_ECHO_START 	0xE000
#define INTERNAL_RAM_START      	0xC000
#define RAM_BANK_SWTC_START     	0xA000
#define VIDEO_RAM_START         	0x8000
#define ROM_BANK_SWTC_START 		0x4000
#define ROM_BANK_0_START 			0x0000

#define HIGH_RAM_SIZE               0x007F
#define IO_MAPPED_SIZE              0x004C
#define OAM_RAM_SIZE                0x00A0
#define INTERNAL_RAM_ECHO_SIZE      0x1E00
#define INTERNAL_RAM_SIZE      		0x2000
#define RAM_BANK_SWTC_SIZE     		0x2000
#define VIDEO_RAM_SIZE        		0x2000
#define ROM_BANK_SWTC_SIZE 			0x4000
#define ROM_BANK_0_SIZE 			0x4000

#define BOOT_ROM_SIZE               0x100

#define ROM_BANK_SIZE               0x4000

class Memory {
private:

    uint8_t mbc1RomBankNumber = 1;

    uint8_t *memory;
    uint8_t *bootRom;
    uint8_t *gameRom;

    bool reading;
    bool traceEnabled;

    unordered_map<uint16_t, IoDevice *> ioMap; 

	uint8_t *getMemoryAreaForAddress(uint16_t *address);

public:
	Memory(uint8_t *bootRom, uint8_t* gameRom);
	~Memory();

    void registerIoDevice(uint16_t, IoDevice *);

	uint16_t read16(uint16_t address, bool trace = true);
	uint8_t read8(uint16_t address, bool trace = true);

	void write16(uint16_t address, uint16_t value, bool trace = true);
	void write8(uint16_t address, uint8_t value, bool trace = true);

    bool bootRomEnabled();

    void *getRawPointer(uint16_t address);
};

#endif