
#include <iostream>

#include "Cpu.hpp"
#include "LogUtil.hpp"

using namespace std;

Cpu::Cpu(Memory *memory) {
    this->memory = memory;
}

void Cpu::push8(uint8_t val) {
    TRACE_STACK_OP(endl)
    TRACE_STACK_OP("Push 8 bit value SP=" << cout16Hex(regSP) << " -> " << cout8Hex(val));
    memory->write8(regSP, val);
    regSP--;
}

void Cpu::push16(uint16_t val) {
    push8(msbOf(val));
    push8(lsbOf(val));
}

uint8_t Cpu::pop8() {
    regSP++;
    TRACE_STACK_OP(endl)
    TRACE_STACK_OP("Pop 8 bit value SP=" << cout16Hex(regSP) << " <- " << cout8Hex(memory->read8(regSP)));
    return memory->read8(regSP);
}

uint16_t Cpu::pop16() {
    uint8_t lsb = pop8();
    uint8_t msb = pop8();
    return littleEndianCombine(lsb, msb);
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
    setReg##REG(reg##REG() + 1);                                                      \
    OPCODE_INC_8_BIT_FLAGS(reg##REG());                                               \
    ADD_CYCLES(4);                                                                    \
    break;                                                                            \
}
#define OPCODE_INC_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_PFX << "INC (" << #REGPTR << ")");                               \
    uint8_t val = memory->read8(reg##REGPTR) + 1;                                     \
    memory->write8(reg##REGPTR, val);                                                 \
    OPCODE_INC_8_BIT_FLAGS(val);                                                      \
    ADD_CYCLES(12);                                                                   \
    break;                                                                            \
}
#define OPCODE_INC_REG_16_BIT(REG) {                                                  \
    TRACE_CPU(OPCODE_PFX << "INC " << #REG);                                          \
    reg##REG++;                                                                       \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_REGPTR_REG_8_BIT(REGPTR, REG) {                                     \
    TRACE_CPU(OPCODE_PFX << "LD (" << #REGPTR << ")," << #REG);                       \
    memory->write8(reg##REGPTR, reg##REG());                                          \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_IMM16PTR_REG_8_BIT(REG) {                                           \
    uint16_t arg = imm16();                                                           \
    TRACE_CPU(OPCODE_PFX << "LD (" << cout16Hex(arg) << ")," << #REG);                \
    memory->write8(arg, reg##REG());                                                  \
    ADD_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_LD_REG_REGPTR_8_BIT(REGD, REGPTR) {                                    \
    TRACE_CPU(OPCODE_PFX << "LD " << #REGD << ",(" << #REGPTR << ")");                \
    setReg##REGD(memory->read8(reg##REGPTR));                                         \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_REG_REG_8_BIT(REGD, REGS) {                                         \
    TRACE_CPU(OPCODE_PFX << "LD " << #REGD << "," << #REGS);                          \
    setReg##REGD(reg##REGS());                                                        \
    ADD_CYCLES(4);                                                                    \
    break;                                                                            \
}

// TODO check the borrow and half borrow logic 
#define OPCODE_CP_8_BIT_FLAGS(N) {                                                    \
    setOrClearFlag(FLAG_ZERO, regA() == N);                                           \
    setFlag(FLAG_SUBTRACT);                                                           \
    setOrClearFlag(FLAG_HALF_CARRY, lowNibbleOf(regA()) < lowNibbleOf(N));            \
    setOrClearFlag(FLAG_CARRY, regA() < N);                                           \
}
#define OPCODE_CP_N_8_BIT() {                                                         \
    uint8_t arg = imm8();                                                             \
    TRACE_CPU(OPCODE_PFX << "CP " << cout8Hex(arg));                                  \
    OPCODE_CP_8_BIT_FLAGS(arg);                                                       \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_PUSH_REG_16(REG) {                                                     \
    TRACE_CPU(OPCODE_PFX << "PUSH " << #REG);                                         \
    push16(reg##REG);                                                                 \
    ADD_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_POP_REG_16(REG) {                                                      \
    TRACE_CPU(OPCODE_PFX << "POP " << #REG);                                          \
    reg##REG = pop16();                                                               \
    ADD_CYCLES(12);                                                                   \
    break;                                                                            \
}

#define OPCODE_RL_8_BIT_FLAGS(BEFORE, AFTER) {                                        \
    setOrClearFlag(FLAG_CARRY, isBitSet(BEFORE, 7));                                  \
    setOrClearFlag(FLAG_ZERO, AFTER == 0);                                            \
}

// Rotate left through carry (unlike RLC)
#define OPCODE_RL_REG_8_BIT(REG) {                                                    \
    TRACE_CPU(OPCODE_CB_PFX << "RL " << #REG);                                        \
    uint8_t before = reg##REG();                                                      \
    setReg##REG((before << 1) | flag(FLAG_CARRY));                                    \
    OPCODE_RL_8_BIT_FLAGS(before, reg##REG());                                        \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

// Rotate left through carry (unlike RLC)
#define OPCODE_RL_REGPTR_8_BIT(REGPTR) {                                              \
    TRACE_CPU(OPCODE_CB_PFX << "RL (" << #REGPTR << ")");                             \
    uint8_t before = memory->read8(reg##REGPTR);                                      \
    uint8_t after = (before << 1) | flag(FLAG_CARRY);                                 \
    memory->write8(reg##REGPTR, after);                                               \
    OPCODE_RL_8_BIT_FLAGS(before, after);                                             \
    ADD_CYCLES(16);                                                                   \
    break;                                                                            \
}

// Rotate left through carry (unlike RLC)
#define OPCODE_RLA() {                                                                \
    TRACE_CPU(OPCODE_PFX << "RLA");                                                   \
    uint8_t before = regA();                                                          \
    setRegA((before << 1) | flag(FLAG_CARRY));                                        \
    OPCODE_RL_8_BIT_FLAGS(before, regA());                                            \
    ADD_CYCLES(4);                                                                    \
    break;                                                                            \
}

// TODO check carry logic
#define OPCODE_ADC_8_BIT(OP2) {                                                       \
    uint16_t op1 = regA();                                                            \
    uint16_t op2 = OP2;                                                               \
    uint16_t carry = flag(FLAG_CARRY) ? 1 : 0;                                        \
    uint16_t res = op1 + op2 + carry;                                                 \
    setOrClearFlag(FLAG_ZERO, res == 0);                                              \
    clearFlag(FLAG_SUBTRACT);                                                         \
    setOrClearFlag(FLAG_HALF_CARRY,                                                   \
        ((lowNibbleOf(op1) + lowNibbleOf(op2) + carry) & 0xF0) == 0xF0);              \
    setOrClearFlag(FLAG_HALF_CARRY,                                                   \
        ((op1 + op2 + carry) & 0xF00) == 0xF00);                                      \
}

#define OPCODE_ADC_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_PFX << "ADC A," << #REG);                                        \
    OPCODE_ADC_8_BIT(reg##REG());                                                     \
    ADD_CYCLES(4);                                                                    \
    break;                                                                            \
}

#define OPCODE_ADC_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_PFX << "ADC A,(" << #REGPTR << ")");                             \
    OPCODE_ADC_8_BIT(memory->read8(reg##REGPTR));                                     \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_ADC_IMM_8_BIT() {                                                      \
    uint8_t arg = imm8();                                                             \
    TRACE_CPU(OPCODE_PFX << "ADC A," << cout8Hex(arg));                               \
    OPCODE_ADC_8_BIT(arg);                                                            \
    ADD_CYCLES(8);                                                                    \
    break;                                                                            \
}

void Cpu::dumpStatus() {
    TRACE_CPU("[A: " << cout8Hex(regA()) << " B: " << cout8Hex(regB()) << " C: " << cout8Hex(regC()));
    TRACE_CPU(" D: " << cout8Hex(regD()) << " E: " << cout8Hex(regE()) << " H: " << cout8Hex(regH()));
    TRACE_CPU(" F: " << cout8Hex(regF()) << " L: " << cout8Hex(regL()) << " AF: " << cout16Hex(regAF));
    TRACE_CPU(" BC: " << cout16Hex(regBC) << " DE: " << cout16Hex(regDE) << " HL: " << cout16Hex(regHL) << "] ");
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
        // INC DE
        case 0x13: OPCODE_INC_REG_16_BIT(DE);
        // INC BC
        case 0x03: OPCODE_INC_REG_16_BIT(BC);
        // INC HL
        case 0x23: OPCODE_INC_REG_16_BIT(HL);
        // INC SP
        case 0x33: OPCODE_INC_REG_16_BIT(SP);
        // INC (HL)
        case 0x34: OPCODE_INC_REGPTR_8_BIT(HL);
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
        case 0x77: OPCODE_LD_REGPTR_REG_8_BIT(HL, A);
        // LD (HL), B
        case 0x70: OPCODE_LD_REGPTR_REG_8_BIT(HL, B);
        // LD (HL), C
        case 0x71: OPCODE_LD_REGPTR_REG_8_BIT(HL, C);
        // LD (HL), D
        case 0x72: OPCODE_LD_REGPTR_REG_8_BIT(HL, D);
        // LD (HL), E
        case 0x73: OPCODE_LD_REGPTR_REG_8_BIT(HL, E);
        // LD (HL), H
        case 0x74: OPCODE_LD_REGPTR_REG_8_BIT(HL, H);
        // LD (HL), L
        case 0x75: OPCODE_LD_REGPTR_REG_8_BIT(HL, L);
        // LD (BC), A
        case 0x02: OPCODE_LD_REGPTR_REG_8_BIT(BC, A);
        // LD (DE), A
        case 0x12: OPCODE_LD_REGPTR_REG_8_BIT(DE, A);
        // LD (nn), A
        case 0xEA: OPCODE_LD_IMM16PTR_REG_8_BIT(A);
        // LD A,(HL)
        case 0x7E: OPCODE_LD_REG_REGPTR_8_BIT(A, HL);
        // LD B,(HL)
        case 0x46: OPCODE_LD_REG_REGPTR_8_BIT(B, HL);
        // LD C,(HL)
        case 0x4E: OPCODE_LD_REG_REGPTR_8_BIT(C, HL);
        // LD D,(HL)
        case 0x56: OPCODE_LD_REG_REGPTR_8_BIT(D, HL);
        // LD E,(HL)
        case 0x5E: OPCODE_LD_REG_REGPTR_8_BIT(E, HL);
        // LD H,(HL)
        case 0x66: OPCODE_LD_REG_REGPTR_8_BIT(H, HL);
        // LD L,(HL)
        case 0x6E: OPCODE_LD_REG_REGPTR_8_BIT(L, HL);
        // LD A,(DE)
        case 0x1A: OPCODE_LD_REG_REGPTR_8_BIT(A, DE);
        // LD A,(BC)
        case 0x0A: OPCODE_LD_REG_REGPTR_8_BIT(A, BC);
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
        // CP n
        case 0xFE: OPCODE_CP_N_8_BIT();
        // PUSH AF
        case 0xF5: OPCODE_PUSH_REG_16(AF);
        // PUSH BC
        case 0xC5: OPCODE_PUSH_REG_16(BC);
        // PUSH DE
        case 0xD5: OPCODE_PUSH_REG_16(DE);
        // PUSH HL
        case 0xE5: OPCODE_PUSH_REG_16(HL);
        // POP AF
        case 0xF1: OPCODE_POP_REG_16(AF);
        // POP BC
        case 0xC1: OPCODE_POP_REG_16(BC);
        // POP DE
        case 0xD1: OPCODE_POP_REG_16(DE);
        // POP HL
        case 0xE1: OPCODE_POP_REG_16(HL);
        // RLA
        case 0x17: OPCODE_RLA();
        // ADC A,A
        case 0x8F: OPCODE_ADC_REG_8_BIT(A);
        // ADC A,B
        case 0x88: OPCODE_ADC_REG_8_BIT(B);
        // ADC A,C
        case 0x89: OPCODE_ADC_REG_8_BIT(C);
        // ADC A,D
        case 0x8A: OPCODE_ADC_REG_8_BIT(D);
        // ADC A,E
        case 0x8B: OPCODE_ADC_REG_8_BIT(E);
        // ADC A,H
        case 0x8C: OPCODE_ADC_REG_8_BIT(H);
        // ADC A,L
        case 0x8D: OPCODE_ADC_REG_8_BIT(L);
        // ADC A,(HL)
        case 0x8E: OPCODE_ADC_REGPTR_8_BIT(HL);
        // ADC A,n
        case 0xCE: OPCODE_ADC_IMM_8_BIT();

        case 0x90:
        // SUB B
        TRACE_CPU("Unimplemented SUB B");
        unimplemented = true;
        break;

        // LD (HL-),A
        case 0x32:
            TRACE_CPU(OPCODE_PFX << "LD (HL-),A");
            memory->write8(regHL--, regA());
            ADD_CYCLES(8);
            break;

        // RET
        case 0xC9: {
            TRACE_CPU(OPCODE_PFX << "RET");
            regPC = pop16();
            ADD_CYCLES(8);
            break;
        }
        // LD ($FF00+n),A
        case 0xE0: {
            uint8_t arg = imm8();
            TRACE_CPU(OPCODE_PFX << "LD ($FF00+" << cout8Hex(arg) << "),A");
            memory->write8(0xFF00+arg, regA());
            ADD_CYCLES(12);
            break;
        }
        // LD A,($FF00+n)
        case 0xF0: {
            uint8_t arg = imm8();
            TRACE_CPU(OPCODE_PFX << "LD A, ($FF00+" << cout8Hex(arg) << ")");
            setRegA(memory->read8(0xFF00+arg));
            ADD_CYCLES(12);
            break;
        }
        // CALL nn
        case 0xCD: {
            uint16_t dest = imm16();
            TRACE_CPU(OPCODE_PFX << "CALL " << cout16Hex(dest));
            push16(regPC);
            regPC = dest;
            ADD_CYCLES(12);
            break;
        }
        // LD ($FF00+C),A
        case 0xE2: {
            TRACE_CPU(OPCODE_PFX << "LD ($FF00+C),A");
            memory->write8(0xFF00+regC(), regA());
            ADD_CYCLES(8);
            break;
        }
        // LD (HL+), A
        case 0x22: {
            TRACE_CPU(OPCODE_PFX << "LD (HL+), A");
            memory->write8(regHL, regA());
            regHL++;
            ADD_CYCLES(8);
            break;
        }
        // CB Prefix
        case 0xCB:
        opcode = memory->read8(regPC++);
        TRACE_CPU(cout8Hex(opcode));
        switch (opcode) {
            // BIT 7,H
            case 0x7C: OPCODE_BIT_REG_8_BIT(7, H);
            // RL A
            case 0x17: OPCODE_RL_REG_8_BIT(A);
            // RL B
            case 0x10: OPCODE_RL_REG_8_BIT(B);
            // RL C
            case 0x11: OPCODE_RL_REG_8_BIT(C);
            // RL D
            case 0x12: OPCODE_RL_REG_8_BIT(D);
            // RL E
            case 0x13: OPCODE_RL_REG_8_BIT(E);
            // RL H
            case 0x14: OPCODE_RL_REG_8_BIT(H);
            // RL L
            case 0x15: OPCODE_RL_REG_8_BIT(L);
            // RL (HL)
            case 0x16: OPCODE_RL_REGPTR_8_BIT(HL);

            default:
                unimplemented = true;
                break;
        }
        break;

        default:
            TRACE_CPU("Unimplemented!!!");
            unimplemented = true;
            break;
    }

    TRACE_CPU(endl)
}
























