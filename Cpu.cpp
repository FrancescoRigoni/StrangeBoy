
#include <iostream>

#include "Cpu.hpp"
#include "LogUtil.hpp"

using namespace std;

Cpu::Cpu(Memory *memory) {
    this->memory = memory;
}

void Cpu::push8(uint8_t val) {
    memory->write8(regSP, val);
    regSP--;
}

void Cpu::push16(uint16_t val) {
    push8(msbOf(val));
    push8(lsbOf(val));
}

uint8_t Cpu::pop8() {
    regSP++;
    return memory->read8(regSP);
}

uint16_t Cpu::pop16() {
    return littleEndianCombine(pop8(), pop8());
}

uint8_t Cpu::imm8() {
    return memory->read8(regPC++);
}

uint16_t Cpu::imm16() {
    regPC += 2;
    return memory->read16(regPC-2);
}

#define ADD_CYCLES(n) cycles += n

#define OPCODE_DEC_REG_8_BIT(REG) {                                      \
    TRACE_CPU(OPCODE_PFX << "DEC " << #REG);                             \
    setReg##REG(reg##REG()-1);                                           \
    setOrClearFlag(FLAG_ZERO, reg##REG() == 0);                          \
    setOrClearFlag(FLAG_HALF_CARRY, lowNibbleOf(reg##REG()) == 0xF);     \
    setFlag(FLAG_SUBTRACT);                                              \
    ADD_CYCLES(4);                                                       \
    break;                                                               \
}

#define OPCODE_LD_REG_N_8_BIT(REG) {                                     \
    uint8_t arg = imm8();                                                \
    TRACE_CPU(OPCODE_PFX << "LD " << #REG << "," << cout8Hex(arg));      \
    setReg##REG(arg);                                                    \
    ADD_CYCLES(8);                                                       \
    break;                                                               \
}

#define OPCODE_JR_COND(cond, strcond) {                                               \
    int8_t signedOffset = imm8();                                                     \
    TRACE_CPU(OPCODE_PFX << "JR " << #strcond << "," << cout8Signed(signedOffset));   \
    bool eval = cond;                                                                 \
    regPC += eval ? signedOffset : 0;                                                 \
    ADD_CYCLES(eval ? 12 : 8);                                                        \
    break;                                                                            \
}

#define OPCODE_LD_N_NN_16_BIT(REG) {                                                  \
    uint16_t arg = imm16();                                                           \
    TRACE_CPU(OPCODE_PFX << "LD " << #REG << "," << cout16Hex(arg));                  \
    reg##REG = arg;                                                                   \
    ADD_CYCLES(12);                                                                   \
    break;                                                                            \
}

#define OPCODE_XOR_N_8_BIT(REG) {                                                     \
    TRACE_CPU(OPCODE_PFX << "XOR " << #REG);                                          \
    setRegA(regA() ^ reg##REG());                                                     \
    setOrClearFlag(FLAG_ZERO, regA() == 0);                                           \
    clearFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY | FLAG_CARRY);                          \
    ADD_CYCLES(4);                                                                    \
    break;                                                                            \
}

#define OPCODE_BIT_REG_8_BIT(BIT, REG) {                                              \
    TRACE_CPU(OPCODE_CB_PFX << "BIT " << #BIT << "," << #REG);                        \
    setOrClearFlag(FLAG_ZERO, isBitClear(reg##REG(), BIT));                           \
    clearFlag(FLAG_SUBTRACT);                                                         \
    setFlag(FLAG_HALF_CARRY);                                                         \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_INC_8_BIT_FLAGS(value) {                                               \
    setOrClearFlag(FLAG_ZERO, value == 0);                                            \
    setOrClearFlag(FLAG_HALF_CARRY, lowNibbleOf(value) == 0);                         \
    clearFlag(FLAG_SUBTRACT);                                                         \
}
#define OPCODE_INC_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_PFX << "INC " << #REG);                                          \
    setRegC(reg##REG() + 1);                                                          \
    OPCODE_INC_8_BIT_FLAGS(reg##REG());                                               \
    ADD_CYCLES(4);                                                                    \
    break;                                                                            \
}
#define OPCODE_INC_HL_8_BIT() {                                                       \
    TRACE_CPU(OPCODE_PFX << "INC (HL)");                                              \
    uint8_t val = memory->read8(regHL) + 1;                                           \
    memory->write8(regHL, val);                                                       \
    OPCODE_INC_8_BIT_FLAGS(val);                                                      \
    ADD_CYCLES(12);                                                                   \
    break;                                                                            \
}

#define OPCODE_LD_HL_REG_8_BIT(REG) {                                                 \
    TRACE_CPU(OPCODE_PFX << "LD (HL)," << #REG);                                      \
    memory->write8(regHL, reg##REG());                                                \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_REG_HL_8_BIT(REG) {                                                 \
    TRACE_CPU(OPCODE_PFX << "LD " << #REG << ",(HL)");                                \
    setReg##REG(memory->read8(regHL));                                                \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_REG_REG_8_BIT(REGD, REGS) {                                         \
    TRACE_CPU(OPCODE_PFX << "LD " << #REGD << "," << #REGS);                          \
    setReg##REGD(reg##REGS());                                                          \
    ADD_CYCLES(4);                                                                    \
    break;                                                                            \
}

void Cpu::cycle() {
    uint8_t opcode = memory->read8(regPC++);
    TRACE_CPU(cout16Hex(regPC-1) << "  :  " << cout8Hex(opcode));

    switch (opcode) {
        // DEC B
        case 0x05: OPCODE_DEC_REG_8_BIT(B); 
        // DEC D
        case 0x15: OPCODE_DEC_REG_8_BIT(D);
        // DEC A
        case 0x3D: OPCODE_DEC_REG_8_BIT(A);
        // DEC E
        case 0x1D: OPCODE_DEC_REG_8_BIT(E);
        // DEC C
        case 0x0D: OPCODE_DEC_REG_8_BIT(C);
        // INC A
        case 0x3C: OPCODE_INC_REG_8_BIT(A);
        // INC B
        case 0x04: OPCODE_INC_REG_8_BIT(B);
        // INC C
        case 0x0C: OPCODE_INC_REG_8_BIT(C);
        // INC D
        case 0x14: OPCODE_INC_REG_8_BIT(D);
        // INC E
        case 0x1C: OPCODE_INC_REG_8_BIT(E);
        // INC H
        case 0x24: OPCODE_INC_REG_8_BIT(H);
        // INC L
        case 0x2C: OPCODE_INC_REG_8_BIT(L);
        // INC (HL)
        case 0x34: OPCODE_INC_HL_8_BIT();
        // LD A, $n
        case 0x3E: OPCODE_LD_REG_N_8_BIT(A);
        // LD B,$n
        case 0x06: OPCODE_LD_REG_N_8_BIT(B);
        // LD D,$n
        case 0x16: OPCODE_LD_REG_N_8_BIT(D);
        // LD L, $n
        case 0x2E: OPCODE_LD_REG_N_8_BIT(L);
        // LD C, $n
        case 0x0E: OPCODE_LD_REG_N_8_BIT(C);
        // LD E, $n
        case 0x1E: OPCODE_LD_REG_N_8_BIT(E);
        // JR n
        case 0x18: OPCODE_JR_COND(true, ALWAYS);
        // JR NZ, n
        case 0x20: OPCODE_JR_COND(!flag(FLAG_ZERO), NZ);
        // JR Z, n
        case 0x28: OPCODE_JR_COND(flag(FLAG_ZERO), Z);
        // JR NC, n
        case 0x30: OPCODE_JR_COND(!flag(FLAG_CARRY), NC);
        // JR C, n
        case 0x38: OPCODE_JR_COND(flag(FLAG_CARRY), C);
        // LD BC, $n
        case 0x01: OPCODE_LD_N_NN_16_BIT(BC);
        // LD DE, $n
        case 0x11: OPCODE_LD_N_NN_16_BIT(DE);
        // LD HL, $n
        case 0x21: OPCODE_LD_N_NN_16_BIT(HL);
        // LD SP, $n
        case 0x31: OPCODE_LD_N_NN_16_BIT(SP);
        // XOR A
        case 0xAF: OPCODE_XOR_N_8_BIT(A);
        // XOR B
        case 0xA8: OPCODE_XOR_N_8_BIT(B);
        // XOR C
        case 0xA9: OPCODE_XOR_N_8_BIT(C);
        // XOR D
        case 0xAA: OPCODE_XOR_N_8_BIT(D);
        // XOR E
        case 0xAB: OPCODE_XOR_N_8_BIT(E);
        // XOR H
        case 0xAC: OPCODE_XOR_N_8_BIT(H);
        // XOR L
        case 0xAD: OPCODE_XOR_N_8_BIT(L);
        // LD (HL), A
        case 0x77: OPCODE_LD_HL_REG_8_BIT(A);
        // LD (HL), B
        case 0x70: OPCODE_LD_HL_REG_8_BIT(B);
        // LD (HL), C
        case 0x71: OPCODE_LD_HL_REG_8_BIT(C);
        // LD (HL), D
        case 0x72: OPCODE_LD_HL_REG_8_BIT(D);
        // LD (HL), E
        case 0x73: OPCODE_LD_HL_REG_8_BIT(E);
        // LD (HL), H
        case 0x74: OPCODE_LD_HL_REG_8_BIT(H);
        // LD (HL), L
        case 0x75: OPCODE_LD_HL_REG_8_BIT(L);
        // LD A,(HL)
        case 0x7E: OPCODE_LD_REG_HL_8_BIT(A);
        // LD B,(HL)
        case 0x46: OPCODE_LD_REG_HL_8_BIT(B);
        // LD C,(HL)
        case 0x4E: OPCODE_LD_REG_HL_8_BIT(C);
        // LD D,(HL)
        case 0x56: OPCODE_LD_REG_HL_8_BIT(D);
        // LD E,(HL)
        case 0x5E: OPCODE_LD_REG_HL_8_BIT(E);
        // LD H,(HL)
        case 0x66: OPCODE_LD_REG_HL_8_BIT(H);
        // LD L,(HL)
        case 0x6E: OPCODE_LD_REG_HL_8_BIT(L);
        // LD A, A
        case 0x7F: OPCODE_LD_REG_REG_8_BIT(A, A);
        // LD A, B
        case 0x78: OPCODE_LD_REG_REG_8_BIT(A, B);
        // LD A, C
        case 0x79: OPCODE_LD_REG_REG_8_BIT(A, C);
        // LD A, D
        case 0x7A: OPCODE_LD_REG_REG_8_BIT(A, D);
        // LD A, E
        case 0x7B: OPCODE_LD_REG_REG_8_BIT(A, E);
        // LD A, H
        case 0x7C: OPCODE_LD_REG_REG_8_BIT(A, H);
        // LD A, L
        case 0x7D: OPCODE_LD_REG_REG_8_BIT(A, L);
        // LD B, A
        case 0x47: OPCODE_LD_REG_REG_8_BIT(B, A);
        // LD B, B
        case 0x40: OPCODE_LD_REG_REG_8_BIT(B, B);
        // LD B, C
        case 0x41: OPCODE_LD_REG_REG_8_BIT(B, C);
        // LD B, D
        case 0x42: OPCODE_LD_REG_REG_8_BIT(B, D);
        // LD B, E
        case 0x43: OPCODE_LD_REG_REG_8_BIT(B, E);
        // LD B, H
        case 0x44: OPCODE_LD_REG_REG_8_BIT(B, H);
        // LD B, L
        case 0x45: OPCODE_LD_REG_REG_8_BIT(B, L);
        // LD C, A
        case 0x4F: OPCODE_LD_REG_REG_8_BIT(C, A);
        // LD C, B
        case 0x48: OPCODE_LD_REG_REG_8_BIT(C, B);
        // LD C, C
        case 0x49: OPCODE_LD_REG_REG_8_BIT(C, C);
        // LD C, D
        case 0x4A: OPCODE_LD_REG_REG_8_BIT(C, D);
        // LD C, E
        case 0x4B: OPCODE_LD_REG_REG_8_BIT(C, E);
        // LD C, H
        case 0x4C: OPCODE_LD_REG_REG_8_BIT(C, H);
        // LD C, L
        case 0x4D: OPCODE_LD_REG_REG_8_BIT(C, L);
        // LD D, A
        case 0x57: OPCODE_LD_REG_REG_8_BIT(D, A);
        // LD D, B
        case 0x50: OPCODE_LD_REG_REG_8_BIT(D, B);
        // LD D, C
        case 0x51: OPCODE_LD_REG_REG_8_BIT(D, C);
        // LD D, D
        case 0x52: OPCODE_LD_REG_REG_8_BIT(D, D);
        // LD D, E
        case 0x53: OPCODE_LD_REG_REG_8_BIT(D, E);
        // LD D, H
        case 0x54: OPCODE_LD_REG_REG_8_BIT(D, H);
        // LD D, L
        case 0x55: OPCODE_LD_REG_REG_8_BIT(D, L);
        // LD E, A
        case 0x5F: OPCODE_LD_REG_REG_8_BIT(E, A);
        // LD E, B
        case 0x58: OPCODE_LD_REG_REG_8_BIT(E, B);
        // LD E, C
        case 0x59: OPCODE_LD_REG_REG_8_BIT(E, C);
        // LD E, D
        case 0x5A: OPCODE_LD_REG_REG_8_BIT(E, D);
        // LD E, E
        case 0x5B: OPCODE_LD_REG_REG_8_BIT(E, E);
        // LD E, H
        case 0x5C: OPCODE_LD_REG_REG_8_BIT(E, H);
        // LD E, L
        case 0x5D: OPCODE_LD_REG_REG_8_BIT(E, L);
        // LD H, A
        case 0x67: OPCODE_LD_REG_REG_8_BIT(H, A);
        // LD H, B
        case 0x60: OPCODE_LD_REG_REG_8_BIT(H, B);
        // LD H, C
        case 0x61: OPCODE_LD_REG_REG_8_BIT(H, C);
        // LD H, D
        case 0x62: OPCODE_LD_REG_REG_8_BIT(H, D);
        // LD H, E
        case 0x63: OPCODE_LD_REG_REG_8_BIT(H, E);
        // LD H, H
        case 0x64: OPCODE_LD_REG_REG_8_BIT(H, H);
        // LD H, L
        case 0x65: OPCODE_LD_REG_REG_8_BIT(H, L);
        // LD L, A
        case 0x6F: OPCODE_LD_REG_REG_8_BIT(L, A);
        // LD L, B
        case 0x68: OPCODE_LD_REG_REG_8_BIT(L, B);
        // LD L, C
        case 0x69: OPCODE_LD_REG_REG_8_BIT(L, C);
        // LD L, D
        case 0x6A: OPCODE_LD_REG_REG_8_BIT(L, D);
        // LD L, E
        case 0x6B: OPCODE_LD_REG_REG_8_BIT(L, E);
        // LD L, H
        case 0x6C: OPCODE_LD_REG_REG_8_BIT(L, H);
        // LD L, L
        case 0x6D: OPCODE_LD_REG_REG_8_BIT(L, L);

        case 0x22:
        // LD (HL+), A
        TRACE_CPU("Unimplemented LD (HL+), A");
        break;

        case 0x23:
        // INC HL
        TRACE_CPU("Unimplemented INC HL");
        break;

        case 0xE0:
        // LD ($FF00+C),A
        TRACE_CPU("Unimplemented LD ($FF00+" << cout8Hex(imm8()) << "),A");
        break;

        case 0xF0:
        // LD A, ($FF00+$n)
        TRACE_CPU("Unimplemented LD A, ($FF00+" << cout8Hex(imm8()) << ")");
        break;

        case 0xEA:
        // LD ($nn), A
        TRACE_CPU("Unimplemented LD (" << cout16Hex(imm16()) << "),A");
        break;

        case 0x13:
        // INC DE
        TRACE_CPU("Unimplemented INC DE");
        break;

        case 0x1A:
        // LD A, (DE)
        TRACE_CPU("Unimplemented LD A, (DE)");
        break;

        case 0xCD:
        // CALL nn
        TRACE_CPU("Unimplemented CALL " << cout16Hex(imm16()));
        break;

        case 0xFE:
        // CP n
        TRACE_CPU("Unimplemented CP " << cout8Hex(imm8()));
        break;

        case 0x90:
        // SUB B
        TRACE_CPU("Unimplemented SUB B");
        break;

        case 0xC5:
        // PUSH BC
        TRACE_CPU("Unimplemented PUSH BC");
        break;

        case 0xC1:
        // POP BC
        TRACE_CPU("Unimplemented POP BC");
        break;

        case 0xC9:
        // RET
        TRACE_CPU("Unimplemented RET");
        break;

        case 0x17:
        // RLA
        TRACE_CPU("Unimplemented RLA");
        break;

        // LD (HL-),A
        case 0x32:
        TRACE_CPU(OPCODE_PFX << "LD (HL-),A");
        memory->write8(regHL--, regA());
        ADD_CYCLES(8);
        break;

        // LD ($FF00+C),A
        case 0xE2:
        TRACE_CPU(OPCODE_PFX << "LD ($FF00+C),A");
        memory->write8(0xFF00+regC(), regA());
        ADD_CYCLES(8);
        break;

        case 0xCB:
        opcode = memory->read8(regPC++);
        TRACE_CPU(cout8Hex(opcode));
        switch (opcode) {
            // BIT 7,H
            case 0x7C: OPCODE_BIT_REG_8_BIT(7, H);

            case 0x11:
            // RL C
            TRACE_CPU("Unimplemented RL C");
        }
        break;

        default:
        TRACE_CPU("Unimplemented!!!");
        break;
    }

    TRACE_CPU(endl)
}
























