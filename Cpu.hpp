
#ifndef __CPU_H__
#define __CPU_H__

#include <cstdint>

#include "Memory.hpp"

#define PC_INITIAL 0x0000

class Cpu {
private:
	uint16_t regPC;
	uint16_t regSP;
    Memory *memory;

    void push8(uint8_t);
    void push16(uint16_t);
    uint8_t pop8();
    uint16_t pop16();

    uint8_t imm8();
    uint16_t imm16();

public:
    Cpu(Memory *memory);

	void cycle();
};

#endif