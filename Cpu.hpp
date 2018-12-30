
#ifndef __CPU_H__
#define __CPU_H__

#include <cstdint>

#include "Memory.hpp"
#include "ByteUtil.hpp"

#define PC_INITIAL 0x0000

#define FLAG_ZERO           0b10000000
#define FLAG_SUBTRACT       0b01000000
#define FLAG_HALF_CARRY     0b00100000
#define FLAG_CARRY          0b00010000

class Cpu {
private:
	uint16_t regSP;

    uint16_t regAF;
    uint16_t regBC;
    uint16_t regDE;
    uint16_t regHL;

    long cycles = 0;

    Memory *memory;

    void push8(uint8_t);
    void push16(uint16_t);
    uint8_t pop8();
    uint16_t pop16();

    uint8_t imm8();
    uint16_t imm16();

    inline void setOrClearFlag(uint8_t flag, bool setCondition) {
        if (setCondition) setFlag(flag);
        else clearFlag(flag);
    }
    inline void setFlag(uint8_t flag) { setRegF(regF() | flag); }
    inline void clearFlag(uint8_t flag) { setRegF(regF() & ~flag); }
    inline bool flag(uint8_t flag) { return regF() & flag; }

    inline uint8_t regA() { return msbOf(regAF); }
    inline uint8_t regF() { return lsbOf(regAF); }
    inline uint8_t regB() { return msbOf(regBC); }
    inline uint8_t regC() { return lsbOf(regBC); }
    inline uint8_t regD() { return msbOf(regDE); }
    inline uint8_t regE() { return lsbOf(regDE); }
    inline uint8_t regH() { return msbOf(regHL); }
    inline uint8_t regL() { return lsbOf(regHL); }

    inline void setRegA(uint8_t val) { return msbTo(&regAF, val); }
    inline void setRegF(uint8_t val) { return lsbTo(&regAF, val); }
    inline void setRegB(uint8_t val) { return msbTo(&regBC, val); }
    inline void setRegC(uint8_t val) { return lsbTo(&regBC, val); }
    inline void setRegD(uint8_t val) { return msbTo(&regDE, val); }
    inline void setRegE(uint8_t val) { return lsbTo(&regDE, val); }
    inline void setRegH(uint8_t val) { return msbTo(&regHL, val); }
    inline void setRegL(uint8_t val) { return lsbTo(&regHL, val); }

public:
    uint16_t regPC = PC_INITIAL;

    Cpu(Memory *memory);

	void cycle();
};

#endif