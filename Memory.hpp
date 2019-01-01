
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <cstdint>

#define INTERRUPTS_ENABLE_REG       0xFFFF
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

class Memory {
private:
	uint8_t interruptEnableReg;
    uint8_t bootRomDisableReg = 0;

	uint8_t *highRam;
	uint8_t *ioMapped;
	uint8_t *ramOAM;
	uint8_t *internalRam;
	uint8_t *ramBankSwitchable;
	uint8_t *videoRam;
	uint8_t *romBankSwitchable;
	uint8_t *romBank0;
    uint8_t *bootRom;

	uint8_t *decodeAddress(uint16_t * address);

public:
	Memory(uint8_t *bootRom, uint8_t* gameRom);
	~Memory();

	uint16_t read16(uint16_t address);
	uint8_t read8(uint16_t address);

	void write16(uint16_t address, uint16_t value);
	void write8(uint16_t address, uint8_t value);
};

#endif