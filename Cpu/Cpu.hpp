
#ifndef __CPU_H__
#define __CPU_H__

#include <cstdint>

#include "Cpu/Memory.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/InterruptFlags.hpp"

#define PC_INITIAL 0x0000

#define FLAG_ZERO           0b10000000
#define FLAG_SUBTRACT       0b01000000
#define FLAG_HALF_CARRY     0b00100000
#define FLAG_CARRY          0b00010000

#define INTERRUPT_HANDLER_VBLANK   0x0040
#define INTERRUPT_HANDLER_LCDC     0x0048
#define INTERRUPT_HANDLER_TIMER    0x0050
#define INTERRUPT_HANDLER_JOYPAD   0x0060

class Cpu {
private:
	uint16_t regSP;
    uint16_t regAF;
    uint16_t regBC;
    uint16_t regDE;
    uint16_t regHL;

    bool stoppedWaitingForKey = false;
    bool halted = false;
    int interruptMasterEnable = 0;
    long cyclesToSpend = 0;

    Memory *memory;
    InterruptFlags *interruptFlags;

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

    inline void setRegA(uint8_t val) { msbTo(&regAF, val); }
    inline void setRegF(uint8_t val) { lsbTo(&regAF, val); }
    inline void setRegB(uint8_t val) { msbTo(&regBC, val); }
    inline void setRegC(uint8_t val) { lsbTo(&regBC, val); }
    inline void setRegD(uint8_t val) { msbTo(&regDE, val); }
    inline void setRegE(uint8_t val) { lsbTo(&regDE, val); }
    inline void setRegH(uint8_t val) { msbTo(&regHL, val); }
    inline void setRegL(uint8_t val) { lsbTo(&regHL, val); }

    inline void setRegAF(uint16_t val) { regAF = val & 0xFFF0; }
    inline void setRegBC(uint16_t val) { regBC = val; }
    inline void setRegDE(uint16_t val) { regDE = val; }
    inline void setRegHL(uint16_t val) { regHL = val; }
    inline void setRegSP(uint16_t val) { regSP = val; }

    void acknowledgeInterrupts();

public:
    uint16_t regPC = PC_INITIAL;
    bool unimplemented = false;

    Cpu(Memory *, InterruptFlags *);

	void cycle(int);
    void execute();
};

#endif