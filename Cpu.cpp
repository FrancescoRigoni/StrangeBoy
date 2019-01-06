
#include <iostream>

#include "Cpu.hpp"
#include "LogUtil.hpp"
#include "Io.hpp"

using namespace std;

Cpu::Cpu(Memory *memory, InterruptFlags *interruptFlags) {
    this->memory = memory;
    this->interruptFlags = interruptFlags;
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

#define USE_CYCLES(n) {     \
    cycles -= n;            \
}

#define OPCODE_NOP() {                     \
    TRACE_CPU(OPCODE_PFX << "NOP");        \
    USE_CYCLES(4);                         \
    break;                                 \
}

#define OPCODE_DI() {                                       \
    TRACE_CPU(OPCODE_PFX << "DI");                          \
    interruptMasterEnable = false;                          \
    USE_CYCLES(4);                                          \
    break;                                                  \
}

#define OPCODE_EI() {                     \
    TRACE_CPU(OPCODE_PFX << "EI");        \
    interruptMasterEnable = true;         \
    USE_CYCLES(4);                        \
    break;                                \
}

#define OPCODE_CPL() {                    \
    TRACE_CPU(OPCODE_PFX << "CPL");       \
    setRegA(~regA());                     \
    setFlag(FLAG_SUBTRACT);               \
    setFlag(FLAG_HALF_CARRY);             \
    USE_CYCLES(4);                        \
    break;                                \
}

#define OPCODE_RST(address) {                                   \
    TRACE_CPU(OPCODE_PFX << "RST " << cout8Hex(address));       \
    push16(regPC);                                              \
    regPC = address;                                            \
    USE_CYCLES(32);                                             \
    break;                                                      \
}

#define OPCODE_DEC_REG_8_BIT(REG) {                                      \
    TRACE_CPU(OPCODE_PFX << "DEC " << #REG);                             \
    setReg##REG(reg##REG()-1);                                           \
    setOrClearFlag(FLAG_ZERO, reg##REG() == 0);                          \
    setOrClearFlag(FLAG_HALF_CARRY, lowNibbleOf(reg##REG()) == 0xF);     \
    setFlag(FLAG_SUBTRACT);                                              \
    USE_CYCLES(4);                                                       \
    break;                                                               \
}

// TODO split flag logic, reuse from OPCODE_DEC_REG_8_BIT
#define OPCODE_DEC_REGPTR_8_BIT(REGPTR) {                                \
    TRACE_CPU(OPCODE_PFX << "DEC (" << #REGPTR << ")");                  \
    uint8_t arg = memory->read8(reg##REGPTR);                            \
    arg = arg - 1;                                                       \
    memory->write8(reg##REGPTR, arg);                                    \
    setOrClearFlag(FLAG_ZERO, arg == 0);                                 \
    setOrClearFlag(FLAG_HALF_CARRY, lowNibbleOf(arg) == 0xF);            \
    setFlag(FLAG_SUBTRACT);                                              \
    USE_CYCLES(12);                                                      \
    break;                                                               \
}

#define OPCODE_DEC_REG_16_BIT(REG) {                                     \
    TRACE_CPU(OPCODE_PFX << "DEC " << #REG);                             \
    --reg##REG;                                                          \
    USE_CYCLES(8);                                                       \
    break;                                                               \
}

#define OPCODE_JR_COND(cond, strcond) {                                               \
    int8_t signedOffset = imm8();                                                     \
    TRACE_CPU(OPCODE_PFX << "JR " << #strcond << "," << cout8Signed(signedOffset));   \
    bool eval = cond;                                                                 \
    regPC += eval ? signedOffset : 0;                                                 \
    USE_CYCLES(eval ? 12 : 8);                                                        \
    break;                                                                            \
}

#define OPCODE_JP_COND(cond, strcond) {                                               \
    int16_t addr = imm16();                                                           \
    TRACE_CPU(OPCODE_PFX << "JP " << #strcond << "," << cout16Hex(addr));             \
    bool eval = cond;                                                                 \
    if (eval) regPC = addr;                                                           \
    USE_CYCLES(eval ? 16 : 12);                                                       \
    break;                                                                            \
}

#define OPCODE_JP() {                                                                 \
    uint16_t arg = imm16();                                                           \
    TRACE_CPU(OPCODE_PFX << "JP " << cout16Hex(arg));                                 \
    regPC = arg;                                                                      \
    USE_CYCLES(12);                                                                   \
    break;                                                                            \
}

#define OPCODE_JP_HL() {                                                              \
    TRACE_CPU(OPCODE_PFX << "JP (HL)");                                               \
    regPC = regHL;                                                                    \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_N_NN_16_BIT(REG) {                                                  \
    uint16_t arg = imm16();                                                           \
    TRACE_CPU(OPCODE_PFX << "LD " << #REG << "," << cout16Hex(arg));                  \
    reg##REG = arg;                                                                   \
    USE_CYCLES(12);                                                                   \
    break;                                                                            \
}

#define OPCODE_XOR_N_8_BIT(REG) {                                                     \
    TRACE_CPU(OPCODE_PFX << "XOR " << #REG);                                          \
    setRegA(regA() ^ reg##REG());                                                     \
    setOrClearFlag(FLAG_ZERO, regA() == 0);                                           \
    clearFlag(FLAG_SUBTRACT | FLAG_HALF_CARRY | FLAG_CARRY);                          \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}

#define OPCODE_BIT_REG_8_BIT(BIT, REG) {                                              \
    TRACE_CPU(OPCODE_CB_PFX << "BIT " << #BIT << "," << #REG);                        \
    setOrClearFlag(FLAG_ZERO, isBitClear(reg##REG(), BIT));                           \
    clearFlag(FLAG_SUBTRACT);                                                         \
    setFlag(FLAG_HALF_CARRY);                                                         \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_BIT_REGPTR_8_BIT(BIT, REGPTR) {                                        \
    TRACE_CPU(OPCODE_CB_PFX << "BIT " << #BIT << ",(" << #REGPTR << ")");             \
    uint8_t arg = memory->read8(reg##REGPTR);                                         \
    setOrClearFlag(FLAG_ZERO, isBitClear(arg, BIT));                                  \
    clearFlag(FLAG_SUBTRACT);                                                         \
    setFlag(FLAG_HALF_CARRY);                                                         \
    USE_CYCLES(16);                                                                   \
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
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}
#define OPCODE_INC_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_PFX << "INC (" << #REGPTR << ")");                               \
    uint8_t val = memory->read8(reg##REGPTR) + 1;                                     \
    memory->write8(reg##REGPTR, val);                                                 \
    OPCODE_INC_8_BIT_FLAGS(val);                                                      \
    USE_CYCLES(12);                                                                   \
    break;                                                                            \
}
#define OPCODE_INC_REG_16_BIT(REG) {                                                  \
    TRACE_CPU(OPCODE_PFX << "INC " << #REG);                                          \
    ++reg##REG;                                                                       \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_REGPTR_REG_8_BIT(REGPTR, REG) {                                     \
    TRACE_CPU(OPCODE_PFX << "LD (" << #REGPTR << ")," << #REG);                       \
    memory->write8(reg##REGPTR, reg##REG());                                          \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_REGPTR_IMM_8_BIT(REGPTR) {                                          \
    uint8_t arg = imm8();                                                             \
    TRACE_CPU(OPCODE_PFX << "LD (" << #REGPTR << ")," << cout8Hex(arg));              \
    memory->write8(reg##REGPTR, arg);                                                 \
    USE_CYCLES(12);                                                                   \
    break;                                                                            \
}

#define OPCODE_LD_IMM16PTR_REG_8_BIT(REG) {                                           \
    uint16_t arg = imm16();                                                           \
    TRACE_CPU(OPCODE_PFX << "LD (" << cout16Hex(arg) << ")," << #REG);                \
    memory->write8(arg, reg##REG());                                                  \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_LD_REG_REGPTR_8_BIT(REGD, REGPTR) {                                    \
    TRACE_CPU(OPCODE_PFX << "LD " << #REGD << ",(" << #REGPTR << ")");                \
    setReg##REGD(memory->read8(reg##REGPTR));                                         \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_REG_REG_8_BIT(REGD, REGS) {                                         \
    TRACE_CPU(OPCODE_PFX << "LD " << #REGD << "," << #REGS);                          \
    setReg##REGD(reg##REGS());                                                        \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}
#define OPCODE_LD_REG_IMM_8_BIT(REG) {                                                \
    uint8_t arg = imm8();                                                             \
    TRACE_CPU(OPCODE_PFX << "LD " << #REG << "," << cout8Hex(arg));                   \
    setReg##REG(arg);                                                                 \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_LD_REG_IMMADDR(REG) {                                                  \
    uint16_t addr = imm16();                                                          \
    TRACE_CPU(OPCODE_PFX << "LD " << #REG << "(" << cout16Hex(addr) << ")");          \
    setReg##REG(memory->read8(addr));                                                 \
    USE_CYCLES(16);                                                                   \
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
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_CP_REGPTR_8_BIT(REGPTR) {                                              \
    TRACE_CPU(OPCODE_PFX << "CP " << #REGPTR);                                        \
    uint8_t arg = memory->read8(reg##REGPTR);                                         \
    OPCODE_CP_8_BIT_FLAGS(arg);                                                       \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_CP_REG_8_BIT(REG) {                                                    \
    TRACE_CPU(OPCODE_PFX << "CP " << #REG);                                           \
    uint8_t arg = reg##REG();                                                         \
    OPCODE_CP_8_BIT_FLAGS(arg);                                                       \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}

#define OPCODE_PUSH_REG_16(REG) {                                                     \
    TRACE_CPU(OPCODE_PFX << "PUSH " << #REG);                                         \
    push16(reg##REG);                                                                 \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_POP_REG_16(REG) {                                                      \
    TRACE_CPU(OPCODE_PFX << "POP " << #REG);                                          \
    reg##REG = pop16();                                                               \
    USE_CYCLES(12);                                                                   \
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
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

// Rotate left through carry (unlike RLC)
#define OPCODE_RL_REGPTR_8_BIT(REGPTR) {                                              \
    TRACE_CPU(OPCODE_CB_PFX << "RL (" << #REGPTR << ")");                             \
    uint8_t before = memory->read8(reg##REGPTR);                                      \
    uint8_t after = (before << 1) | flag(FLAG_CARRY);                                 \
    memory->write8(reg##REGPTR, after);                                               \
    OPCODE_RL_8_BIT_FLAGS(before, after);                                             \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

// Rotate left through carry (unlike RLC)
#define OPCODE_RLA() {                                                                \
    TRACE_CPU(OPCODE_PFX << "RLA");                                                   \
    uint8_t before = regA();                                                          \
    setRegA((before << 1) | flag(FLAG_CARRY));                                        \
    OPCODE_RL_8_BIT_FLAGS(before, regA());                                            \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}

// TODO check carry logic
#define OPCODE_ADC_8_BIT(OP2) {                                                       \
    uint16_t op1 = regA();                                                            \
    uint16_t op2 = OP2;                                                               \
    uint16_t carry = flag(FLAG_CARRY) ? 1 : 0;                                        \
    uint16_t res = op1 + op2 + carry;                                                 \
    setRegA(res);                                                                     \
    setOrClearFlag(FLAG_ZERO, regA() == 0);                                           \
    clearFlag(FLAG_SUBTRACT);                                                         \
    setOrClearFlag(FLAG_HALF_CARRY,                                                   \
        ((lowNibbleOf(op1) + lowNibbleOf(op2) + carry) & 0x10) == 0x10);              \
    setOrClearFlag(FLAG_CARRY,                                                        \
        ((op1 + op2 + carry) & 0x100) == 0x100);                                      \
}
#define OPCODE_ADC_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_PFX << "ADC A," << #REG);                                        \
    OPCODE_ADC_8_BIT(reg##REG());                                                     \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}
#define OPCODE_ADC_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_PFX << "ADC A,(" << #REGPTR << ")");                             \
    OPCODE_ADC_8_BIT(memory->read8(reg##REGPTR));                                     \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_ADC_IMM_8_BIT() {                                                      \
    uint8_t arg = imm8();                                                             \
    TRACE_CPU(OPCODE_PFX << "ADC A," << cout8Hex(arg));                               \
    OPCODE_ADC_8_BIT(arg);                                                            \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

// TODO check carry logic
#define OPCODE_ADD_8_BIT(OP2) {                                                       \
    uint16_t op1 = regA();                                                            \
    uint16_t op2 = OP2;                                                               \
    uint16_t res = op1 + op2;                                                         \
    setRegA(res);                                                                     \
    setOrClearFlag(FLAG_ZERO, regA() == 0);                                           \
    clearFlag(FLAG_SUBTRACT);                                                         \
    setOrClearFlag(FLAG_HALF_CARRY,                                                   \
        ((lowNibbleOf(op1) + lowNibbleOf(op2)) & 0x10) == 0x10);                      \
    setOrClearFlag(FLAG_CARRY,                                                        \
        ((op1 + op2) & 0x100) == 0x100);                                              \
}
#define OPCODE_ADD_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_PFX << "ADD A," << #REG);                                        \
    OPCODE_ADD_8_BIT(reg##REG());                                                     \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}
#define OPCODE_ADD_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_PFX << "ADD A,(" << #REGPTR << ")");                             \
    OPCODE_ADD_8_BIT(memory->read8(reg##REGPTR));                                     \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_ADD_IMM_8_BIT() {                                                      \
    uint8_t arg = imm8();                                                             \
    TRACE_CPU(OPCODE_PFX << "ADD A," << cout8Hex(arg));                               \
    OPCODE_ADD_8_BIT(arg);                                                            \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

// TODO check borrow and half borrow logic
#define OPCODE_SUB_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_PFX << "SUB " << #REG);                                          \
    uint8_t result = regA() - reg##REG();                                             \
    setOrClearFlag(FLAG_ZERO, regA() == reg##REG());                                  \
    setOrClearFlag(FLAG_HALF_CARRY, lowNibbleOf(regA()) >= lowNibbleOf(reg##REG()));  \
    setOrClearFlag(FLAG_CARRY, regA() >= reg##REG());                                 \
    setRegA(result);                                                                  \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}


// TODO check carry logic, half carry calculated from bit 11 WTF?
#define OPCODE_ADD_HL_16_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_PFX << "ADD HL," << #REG);                                       \
    uint32_t op1 = regHL;                                                             \
    uint32_t op2 = reg##REG;                                                          \
    uint32_t res = op1 + op2;                                                         \
    regHL = res;                                                                      \
    clearFlag(FLAG_SUBTRACT);                                                         \
    uint16_t bit11Check = 0x1000;                                                     \
    setOrClearFlag(FLAG_HALF_CARRY,                                                   \
        (((uint16_t)lsbOf(op1) + (uint16_t)lsbOf(op2)) & bit11Check) == bit11Check);  \
    setOrClearFlag(FLAG_CARRY,                                                        \
        ((op1 + op2) & 0x10000) == 0x10000);                                          \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_OR_8_BIT_FLAGS() {                                                     \
    setOrClearFlag(FLAG_ZERO, regA() == 0);                                           \
    clearFlag(FLAG_SUBTRACT);                                                         \
    clearFlag(FLAG_HALF_CARRY);                                                       \
    clearFlag(FLAG_CARRY);                                                            \
}
#define OPCODE_OR_REG_8_BIT(REG) {                                                    \
    TRACE_CPU(OPCODE_PFX << "OR " << #REG);                                           \
    setRegA(regA() | reg##REG());                                                     \
    OPCODE_OR_8_BIT_FLAGS();                                                          \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}
#define OPCODE_OR_REGPTR_8_BIT(REGPTR) {                                              \
    TRACE_CPU(OPCODE_PFX << "OR (" << #REGPTR << ")");                                \
    uint8_t arg = memory->read8(reg##REGPTR);                                         \
    setRegA(regA() | arg);                                                            \
    OPCODE_OR_8_BIT_FLAGS();                                                          \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_OR_IMM_8_BIT() {                                                       \
    uint8_t arg = imm8();                                                             \
    TRACE_CPU(OPCODE_PFX << "OR " << cout8Hex(arg));                                  \
    setRegA(regA() | arg);                                                            \
    OPCODE_OR_8_BIT_FLAGS();                                                          \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_AND_8_BIT_FLAGS() {                                                    \
    setOrClearFlag(FLAG_ZERO, regA() == 0);                                           \
    clearFlag(FLAG_SUBTRACT);                                                         \
    setFlag(FLAG_HALF_CARRY);                                                         \
    clearFlag(FLAG_CARRY);                                                            \
}
#define OPCODE_AND_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_PFX << "AND " << #REG);                                          \
    setRegA(regA() & reg##REG());                                                     \
    OPCODE_AND_8_BIT_FLAGS();                                                         \
    USE_CYCLES(4);                                                                    \
    break;                                                                            \
}
#define OPCODE_AND_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_PFX << "AND (" << #REGPTR << ")");                               \
    uint8_t arg = memory->read8(reg##REGPTR);                                         \
    setRegA(regA() & arg);                                                            \
    OPCODE_AND_8_BIT_FLAGS();                                                         \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_AND_IMM_8_BIT() {                                                      \
    uint8_t arg = imm8();                                                             \
    TRACE_CPU(OPCODE_PFX << "AND " << cout8Hex(arg));                                 \
    setRegA(regA() & arg);                                                            \
    OPCODE_AND_8_BIT_FLAGS();                                                         \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}

#define OPCODE_SWAP_8_BIT_FLAGS(result) {                                             \
    clearFlag(FLAG_SUBTRACT);                                                         \
    clearFlag(FLAG_HALF_CARRY);                                                       \
    clearFlag(FLAG_CARRY);                                                            \
    setOrClearFlag(FLAG_ZERO, result == 0);                                           \
}
#define OPCODE_SWAP_REG_8_BIT(REG) {                                                  \
    TRACE_CPU(OPCODE_CB_PFX << "SWAP " << #REG);                                      \
    setReg##REG((lowNibbleOf(reg##REG()) << 4) | highNibbleOf(reg##REG()));           \
    OPCODE_SWAP_8_BIT_FLAGS(reg##REG());                                              \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_SWAP_REGPTR_8_BIT(REGPTR) {                                            \
    TRACE_CPU(OPCODE_CB_PFX << "SWAP (" << #REGPTR << ")");                           \
    uint8_t arg = memory->read8(reg##REGPTR);                                         \
    arg = (lowNibbleOf(arg) << 4) | highNibbleOf(arg);                                \
    memory->write8(reg##REGPTR, arg);                                                 \
    OPCODE_SWAP_8_BIT_FLAGS(arg);                                                     \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_RES_REG_8_BIT(BIT, REG) {                                              \
    TRACE_CPU(OPCODE_CB_PFX << "RES " << cout8Hex(BIT) << "," << #REG);               \
    uint8_t val = reg##REG();                                                         \
    resetBit(BIT, &val);                                                              \
    setReg##REG(val);                                                                 \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_RES_REGPTR_8_BIT(BIT, REGPTR) {                                        \
    TRACE_CPU(OPCODE_CB_PFX << "RES " << cout8Hex(BIT) << ",(" << #REGPTR << ")");    \
    uint8_t val = memory->read8(reg##REGPTR);                                         \
    resetBit(BIT, &val);                                                              \
    memory->write8(reg##REGPTR, val);                                                 \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_SET_REG_8_BIT(BIT, REG) {                                              \
    TRACE_CPU(OPCODE_CB_PFX << "SET " << cout8Hex(BIT) << "," << #REG);               \
    uint8_t val = reg##REG();                                                         \
    setBit(BIT, &val);                                                                \
    setReg##REG(val);                                                                 \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_SET_REGPTR_8_BIT(BIT, REGPTR) {                                        \
    TRACE_CPU(OPCODE_CB_PFX << "SET " << cout8Hex(BIT) << ",(" << #REGPTR << ")");    \
    uint8_t val = memory->read8(reg##REGPTR);                                         \
    setBit(BIT, &val);                                                                \
    memory->write8(reg##REGPTR, val);                                                 \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_RET_COND(COND, CONDSTR) {                                              \
    TRACE_CPU(OPCODE_PFX << "RET " << #CONDSTR << endl);                              \
    bool eval = COND;                                                                 \
    if (eval) {                                                                       \
        regPC = pop16();                                                              \
    }                                                                                 \
    USE_CYCLES(eval ? 20 : 8);                                                        \
    break;                                                                            \
}

#define OPCODE_SLA_8_BIT_FLAGS(before, after) {                                       \
    setOrClearFlag(FLAG_CARRY, isBitSet(before, 7));                                  \
    clearFlag(FLAG_SUBTRACT);                                                         \
    clearFlag(FLAG_HALF_CARRY);                                                       \
    setOrClearFlag(FLAG_ZERO, after == 0);                                            \
}
#define OPCODE_SLA_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_CB_PFX << "SLA " << #REG);                                       \
    uint8_t before = reg##REG();                                                      \
    uint8_t after = before << 1;                                                      \
    setReg##REG(after);                                                               \
    OPCODE_SLA_8_BIT_FLAGS(before, after);                                            \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_SLA_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_CB_PFX << "SLA (" << #REGPTR << ")");                            \
    uint8_t before = memory->read8(reg##REGPTR);                                      \
    uint8_t after = before << 1;                                                      \
    memory->write8(reg##REGPTR, after);                                               \
    OPCODE_SLA_8_BIT_FLAGS(before, after);                                            \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_SRA_8_BIT_FLAGS(before, after) {                                       \
    setOrClearFlag(FLAG_CARRY, isBitSet(before, 0));                                  \
    clearFlag(FLAG_SUBTRACT);                                                         \
    clearFlag(FLAG_HALF_CARRY);                                                       \
    setOrClearFlag(FLAG_ZERO, after == 0);                                            \
}
#define OPCODE_SRA_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_CB_PFX << "SRA " << #REG);                                       \
    uint8_t before = reg##REG();                                                      \
    uint8_t after = before >> 1;                                                      \
    if (isBitSet(before, 7)) after |= 0b10000000;                                     \
    setReg##REG(after);                                                               \
    OPCODE_SRA_8_BIT_FLAGS(before, after);                                            \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_SRA_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_CB_PFX << "SRA (" << #REGPTR << ")");                            \
    uint8_t before = memory->read8(reg##REGPTR);                                      \
    uint8_t after = before >> 1;                                                      \
    if (isBitSet(before, 7)) after |= 0b10000000;                                     \
    memory->write8(reg##REGPTR, after);                                               \
    OPCODE_SRA_8_BIT_FLAGS(before, after);                                            \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

#define OPCODE_SRL_8_BIT_FLAGS(before, after) {                                       \
    setOrClearFlag(FLAG_CARRY, (before & 0x1) == 0x1);                                \
    setOrClearFlag(FLAG_ZERO, after == 0);                                            \
    clearFlag(FLAG_SUBTRACT);                                                         \
    clearFlag(FLAG_HALF_CARRY);                                                       \
}
#define OPCODE_SRL_REG_8_BIT(REG) {                                                   \
    TRACE_CPU(OPCODE_CB_PFX << "SRL " << #REG);                                       \
    uint8_t before = reg##REG();                                                      \
    uint8_t after = before >> 1;                                                      \
    setReg##REG(after);                                                               \
    OPCODE_SRL_8_BIT_FLAGS(before, after);                                            \
    USE_CYCLES(8);                                                                    \
    break;                                                                            \
}
#define OPCODE_SRL_REGPTR_8_BIT(REGPTR) {                                             \
    TRACE_CPU(OPCODE_CB_PFX << "SRL (" << #REGPTR << ")");                            \
    uint8_t before = memory->read8(reg##REGPTR);                                      \
    uint8_t after = before >> 1;                                                      \
    memory->write8(reg##REGPTR, after);                                               \
    OPCODE_SRL_8_BIT_FLAGS(before, after);                                            \
    USE_CYCLES(16);                                                                   \
    break;                                                                            \
}

void Cpu::dumpStatus() {
    TRACE_CPU("[A: " << cout8Hex(regA()) << " B: " << cout8Hex(regB()) << " C: " << cout8Hex(regC()));
    TRACE_CPU(" D: " << cout8Hex(regD()) << " E: " << cout8Hex(regE()) << " H: " << cout8Hex(regH()));
    TRACE_CPU(" F: " << cout8Hex(regF()) << " L: " << cout8Hex(regL()) << " AF: " << cout16Hex(regAF));
    TRACE_CPU(" BC: " << cout16Hex(regBC) << " DE: " << cout16Hex(regDE) << " HL: " << cout16Hex(regHL) << "] ");
}

void Cpu::acknowledgeInterrupts() {
    if (interruptFlags->acknowledgeVBlankInterrupt()) {
        interruptMasterEnable = false;
        push16(regPC);
        regPC = INTERRUPT_HANDLER_VBLANK;

    } else if (interruptFlags->acknowledgeLCDCInterrupt()) {
        interruptMasterEnable = false;
        push16(regPC);
        regPC = INTERRUPT_HANDLER_LCDC;
    }
    /*
    Priority:
    V-Blank
    LCDC Status
    Timer Overflow Serial Transfer
    Hi-Lo of P10-P13
    */
}

void Cpu::cycle(int numberOfCycles) {
    // Check for interrupts
    if (interruptMasterEnable) {
        acknowledgeInterrupts();
    }

    cycles = numberOfCycles;
    do {
        execute();
    } while (cycles > 0);
}

void Cpu::execute() {
    uint8_t opcode = memory->read8(regPC++);
    dumpStatus();
    TRACE_CPU(cout16Hex(regPC-1) << "  :  " << cout8Hex(opcode));

    switch (opcode) {
        // NOP
        case 0x00: OPCODE_NOP();
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
        // DEC H
        case 0x25: OPCODE_DEC_REG_8_BIT(H);
        // DEC L
        case 0x2D: OPCODE_DEC_REG_8_BIT(L);
        // DEC BC
        case 0x0B: OPCODE_DEC_REG_16_BIT(BC);
        // DEC DE
        case 0x1B: OPCODE_DEC_REG_16_BIT(DE);
        // DEC HL
        case 0x2B: OPCODE_DEC_REG_16_BIT(HL);
        // DEC SP
        case 0x3B: OPCODE_DEC_REG_16_BIT(SP);
        // DEC (HL)
        case 0x35: OPCODE_DEC_REGPTR_8_BIT(HL);
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
        case 0x3E: OPCODE_LD_REG_IMM_8_BIT(A);
        // LD B,$n
        case 0x06: OPCODE_LD_REG_IMM_8_BIT(B);
        // LD D,$n
        case 0x16: OPCODE_LD_REG_IMM_8_BIT(D);
        // LD L, $n
        case 0x2E: OPCODE_LD_REG_IMM_8_BIT(L);
        // LD C, $n
        case 0x0E: OPCODE_LD_REG_IMM_8_BIT(C);
        // LD E, $n
        case 0x1E: OPCODE_LD_REG_IMM_8_BIT(E);
        // LD H,n
        case 0x26: OPCODE_LD_REG_IMM_8_BIT(H);
        // JP nn
        case 0xC3: OPCODE_JP();
        // JP (HL)
        case 0xE9: OPCODE_JP_HL();
        // JP NZ,nn
        case 0xC2: OPCODE_JP_COND(!flag(FLAG_ZERO), NZ);
        // JP Z,nn
        case 0xCA: OPCODE_JP_COND(flag(FLAG_ZERO), Z);
        // JP NC,nn
        case 0xD2: OPCODE_JP_COND(!flag(FLAG_CARRY), NC);
        // JP C,nn
        case 0xDA: OPCODE_JP_COND(flag(FLAG_CARRY), C);
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
        // LD (HL), n
        case 0x36: OPCODE_LD_REGPTR_IMM_8_BIT(HL);
        // LD (BC), A
        case 0x02: OPCODE_LD_REGPTR_REG_8_BIT(BC, A);
        // LD (DE), A
        case 0x12: OPCODE_LD_REGPTR_REG_8_BIT(DE, A);
        // LD (nn), A
        case 0xEA: OPCODE_LD_IMM16PTR_REG_8_BIT(A);
        // LD A,(HL)
        case 0x7E: OPCODE_LD_REG_REGPTR_8_BIT(A, HL);
        // LD A,(nn)
        case 0xFA: OPCODE_LD_REG_IMMADDR(A);
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
        // CP A
        case 0xBF: OPCODE_CP_REG_8_BIT(A);
        // CP B
        case 0xB8: OPCODE_CP_REG_8_BIT(B);
        // CP C
        case 0xB9: OPCODE_CP_REG_8_BIT(C);
        // CP D
        case 0xBA: OPCODE_CP_REG_8_BIT(D);
        // CP E
        case 0xBB: OPCODE_CP_REG_8_BIT(E);
        // CP H
        case 0xBC: OPCODE_CP_REG_8_BIT(H);
        // CP L
        case 0xBD: OPCODE_CP_REG_8_BIT(L);
        // CP (HL)
        case 0xBE: OPCODE_CP_REGPTR_8_BIT(HL);
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
        // SUB A
        case 0x97: OPCODE_SUB_REG_8_BIT(A);
        // SUB B
        case 0x90: OPCODE_SUB_REG_8_BIT(B);
        // SUB C
        case 0x91: OPCODE_SUB_REG_8_BIT(C);
        // SUB D
        case 0x92: OPCODE_SUB_REG_8_BIT(D);
        // SUB E
        case 0x93: OPCODE_SUB_REG_8_BIT(E);
        // SUB H
        case 0x94: OPCODE_SUB_REG_8_BIT(H);
        // SUB L
        case 0x95: OPCODE_SUB_REG_8_BIT(L);
        // ADD A
        case 0x87: OPCODE_ADD_REG_8_BIT(A);
        // ADD B
        case 0x80: OPCODE_ADD_REG_8_BIT(B);
        // ADD C
        case 0x81: OPCODE_ADD_REG_8_BIT(C);
        // ADD D
        case 0x82: OPCODE_ADD_REG_8_BIT(D);
        // ADD E
        case 0x83: OPCODE_ADD_REG_8_BIT(E);
        // ADD H
        case 0x84: OPCODE_ADD_REG_8_BIT(H);
        // ADD L
        case 0x85: OPCODE_ADD_REG_8_BIT(L);
        // ADD (HL)
        case 0x86: OPCODE_ADD_REGPTR_8_BIT(HL);
        // ADD n
        case 0xC6: OPCODE_ADD_IMM_8_BIT();
        // DI
        case 0xF3: OPCODE_DI();
        // EI
        case 0xFB: OPCODE_EI();
        // OR A
        case 0xB7: OPCODE_OR_REG_8_BIT(A);
        // OR B
        case 0xB0: OPCODE_OR_REG_8_BIT(B);
        // OR C
        case 0xB1: OPCODE_OR_REG_8_BIT(C);
        // OR D
        case 0xB2: OPCODE_OR_REG_8_BIT(D);
        // OR E
        case 0xB3: OPCODE_OR_REG_8_BIT(E);
        // OR H
        case 0xB4: OPCODE_OR_REG_8_BIT(H);
        // OR L
        case 0xB5: OPCODE_OR_REG_8_BIT(L);
        // OR (HL)
        case 0xB6: OPCODE_OR_REGPTR_8_BIT(HL);
        // OR n
        case 0xF6: OPCODE_OR_IMM_8_BIT();
        // CPL
        case 0x2F: OPCODE_CPL();
        // AND A
        case 0xA7: OPCODE_AND_REG_8_BIT(A);
        // AND B
        case 0xA0: OPCODE_AND_REG_8_BIT(B);
        // AND C
        case 0xA1: OPCODE_AND_REG_8_BIT(C);
        // AND D
        case 0xA2: OPCODE_AND_REG_8_BIT(D);
        // AND E
        case 0xA3: OPCODE_AND_REG_8_BIT(E);
        // AND H
        case 0xA4: OPCODE_AND_REG_8_BIT(H);
        // AND L
        case 0xA5: OPCODE_AND_REG_8_BIT(L);
        // AND (HL)
        case 0xA6: OPCODE_AND_REGPTR_8_BIT(HL);
        // AND n
        case 0xE6: OPCODE_AND_IMM_8_BIT();
        // RST 00
        case 0xC7: OPCODE_RST(0x00);
        // RST 08
        case 0xCF: OPCODE_RST(0x08);
        // RST 10
        case 0xD7: OPCODE_RST(0x10);
        // RST 18
        case 0xDF: OPCODE_RST(0x18);
        // RST 20
        case 0xE7: OPCODE_RST(0x20);
        // RST 28
        case 0xEF: OPCODE_RST(0x28);
        // RST 30
        case 0xF7: OPCODE_RST(0x30);
        // RST 38
        case 0xFF: OPCODE_RST(0x38);
        // ADD HL,BC
        case 0x09: OPCODE_ADD_HL_16_BIT(BC);
        // ADD HL,DE
        case 0x19: OPCODE_ADD_HL_16_BIT(DE);
        // ADD HL,HL
        case 0x29: OPCODE_ADD_HL_16_BIT(HL);
        // ADD HL,SP
        case 0x39: OPCODE_ADD_HL_16_BIT(SP);
        // RET NZ
        case 0xC0: OPCODE_RET_COND(!flag(FLAG_ZERO), NZ);
        // RET Z
        case 0xC8: OPCODE_RET_COND(flag(FLAG_ZERO), Z);
        // RET NC
        case 0xD0: OPCODE_RET_COND(!flag(FLAG_CARRY), NC);
        // RET C
        case 0xD8: OPCODE_RET_COND(flag(FLAG_CARRY), C);

        // LD A, (HL+)
        case 0x2A:
            TRACE_CPU(OPCODE_PFX << "LD A, (HL+)");
            setRegA(memory->read8(regHL++));
            USE_CYCLES(8);
            break;

        // LD A, (HL-)
        case 0x3A:
            TRACE_CPU(OPCODE_PFX << "LD A, (HL-)");
            setRegA(memory->read8(regHL--));
            USE_CYCLES(8);
            break;

        // LD (HL-),A
        case 0x32:
            TRACE_CPU(OPCODE_PFX << "LD (HL-),A");
            memory->write8(regHL--, regA());
            USE_CYCLES(8);
            break;

        // RET
        case 0xC9: {
            TRACE_CPU(OPCODE_PFX << "RET");
            regPC = pop16();
            USE_CYCLES(16);
            break;
        }

        // RETI
        case 0xD9: {
            TRACE_CPU(OPCODE_PFX << "RETI");
            regPC = pop16();
            interruptMasterEnable = true;
            USE_CYCLES(16);
            break;
        }

        // LD ($FF00+n),A
        case 0xE0: {
            uint8_t arg = imm8();
            TRACE_CPU(OPCODE_PFX << "LD ($FF00+" << cout8Hex(arg) << "),A");
            memory->write8(0xFF00+arg, regA());
            USE_CYCLES(12);
            break;
        }
        // LD A,($FF00+n)
        case 0xF0: {
            uint8_t arg = imm8();
            TRACE_CPU(OPCODE_PFX << "LD A, ($FF00+" << cout8Hex(arg) << ")");
            setRegA(memory->read8(0xFF00+arg));
            USE_CYCLES(12);
            break;
        }
        // CALL nn
        case 0xCD: {
            uint16_t dest = imm16();
            TRACE_CPU(OPCODE_PFX << "CALL " << cout16Hex(dest));
            push16(regPC);
            regPC = dest;
            USE_CYCLES(12);
            break;
        }
        // LD ($FF00+C),A
        case 0xE2: {
            TRACE_CPU(OPCODE_PFX << "LD ($FF00+C),A");
            memory->write8(0xFF00+regC(), regA());
            USE_CYCLES(8);
            break;
        }
        // LD (HL+), A
        case 0x22: {
            TRACE_CPU(OPCODE_PFX << "LD (HL+), A");
            memory->write8(regHL, regA());
            regHL++;
            USE_CYCLES(8);
            break;
        }
        // CB Prefix
        case 0xCB:
        opcode = memory->read8(regPC++);
        TRACE_CPU(cout8Hex(opcode));
        switch (opcode) {
            // BIT 0,B
            case 0x40: OPCODE_BIT_REG_8_BIT(0, B);
            // BIT 0,C
            case 0x41: OPCODE_BIT_REG_8_BIT(0, C);
            // BIT 0,D
            case 0x42: OPCODE_BIT_REG_8_BIT(0, D);
            // BIT 0,E
            case 0x43: OPCODE_BIT_REG_8_BIT(0, E);
            // BIT 0,H
            case 0x44: OPCODE_BIT_REG_8_BIT(0, H);
            // BIT 0,L
            case 0x45: OPCODE_BIT_REG_8_BIT(0, L);
            // BIT 0,(HL)
            case 0x46: OPCODE_BIT_REGPTR_8_BIT(0, HL);
            // BIT 0,A
            case 0x47: OPCODE_BIT_REG_8_BIT(0, A);
            // BIT 1,B
            case 0x48: OPCODE_BIT_REG_8_BIT(1, B);
            // BIT 1,C
            case 0x49: OPCODE_BIT_REG_8_BIT(1, C);
            // BIT 1,D
            case 0x4A: OPCODE_BIT_REG_8_BIT(1, D);
            // BIT 1,E
            case 0x4B: OPCODE_BIT_REG_8_BIT(1, E);
            // BIT 1,H
            case 0x4C: OPCODE_BIT_REG_8_BIT(1, H);
            // BIT 1,L
            case 0x4D: OPCODE_BIT_REG_8_BIT(1, L);
            // BIT 1,(HL)
            case 0x4E: OPCODE_BIT_REGPTR_8_BIT(1, HL);
            // BIT 1,A
            case 0x4F: OPCODE_BIT_REG_8_BIT(1, A);
            // BIT 2,B
            case 0x50: OPCODE_BIT_REG_8_BIT(2, B);
            // BIT 2,C
            case 0x51: OPCODE_BIT_REG_8_BIT(2, C);
            // BIT 2,D
            case 0x52: OPCODE_BIT_REG_8_BIT(2, D);
            // BIT 2,E
            case 0x53: OPCODE_BIT_REG_8_BIT(2, E);
            // BIT 2,H
            case 0x54: OPCODE_BIT_REG_8_BIT(2, H);
            // BIT 2,L
            case 0x55: OPCODE_BIT_REG_8_BIT(2, L);
            // BIT 2,(HL)
            case 0x56: OPCODE_BIT_REGPTR_8_BIT(2, HL);
            // BIT 2,A
            case 0x57: OPCODE_BIT_REG_8_BIT(2, A);
            // BIT 3,B
            case 0x58: OPCODE_BIT_REG_8_BIT(3, B);
            // BIT 3,C
            case 0x59: OPCODE_BIT_REG_8_BIT(3, C);
            // BIT 3,D
            case 0x5A: OPCODE_BIT_REG_8_BIT(3, D);
            // BIT 3,E
            case 0x5B: OPCODE_BIT_REG_8_BIT(3, E);
            // BIT 3,H
            case 0x5C: OPCODE_BIT_REG_8_BIT(3, H);
            // BIT 3,L
            case 0x5D: OPCODE_BIT_REG_8_BIT(3, L);
            // BIT 3,(HL)
            case 0x5E: OPCODE_BIT_REGPTR_8_BIT(3, HL);
            // BIT 3,A
            case 0x5F: OPCODE_BIT_REG_8_BIT(3, A);
            // BIT 4,B
            case 0x60: OPCODE_BIT_REG_8_BIT(4, B);
            // BIT 4,C
            case 0x61: OPCODE_BIT_REG_8_BIT(4, C);
            // BIT 4,D
            case 0x62: OPCODE_BIT_REG_8_BIT(4, D);
            // BIT 4,E
            case 0x63: OPCODE_BIT_REG_8_BIT(4, E);
            // BIT 4,H
            case 0x64: OPCODE_BIT_REG_8_BIT(4, H);
            // BIT 4,L
            case 0x65: OPCODE_BIT_REG_8_BIT(4, L);
            // BIT 4,(HL)
            case 0x66: OPCODE_BIT_REGPTR_8_BIT(4, HL);
            // BIT 4,A
            case 0x67: OPCODE_BIT_REG_8_BIT(4, A);
            // BIT 5,B
            case 0x68: OPCODE_BIT_REG_8_BIT(5, B);
            // BIT 5,C
            case 0x69: OPCODE_BIT_REG_8_BIT(5, C);
            // BIT 5,D
            case 0x6A: OPCODE_BIT_REG_8_BIT(5, D);
            // BIT 5,E
            case 0x6B: OPCODE_BIT_REG_8_BIT(5, E);
            // BIT 5,H
            case 0x6C: OPCODE_BIT_REG_8_BIT(5, H);
            // BIT 5,L
            case 0x6D: OPCODE_BIT_REG_8_BIT(5, L);
            // BIT 5,(HL)
            case 0x6E: OPCODE_BIT_REGPTR_8_BIT(5, HL);
            // BIT 5,A
            case 0x6F: OPCODE_BIT_REG_8_BIT(5, A);
            // BIT 6,B
            case 0x70: OPCODE_BIT_REG_8_BIT(6, B);
            // BIT 6,C
            case 0x71: OPCODE_BIT_REG_8_BIT(6, C);
            // BIT 6,D
            case 0x72: OPCODE_BIT_REG_8_BIT(6, D);
            // BIT 6,E
            case 0x73: OPCODE_BIT_REG_8_BIT(6, E);
            // BIT 6,H
            case 0x74: OPCODE_BIT_REG_8_BIT(6, H);
            // BIT 6,L
            case 0x75: OPCODE_BIT_REG_8_BIT(6, L);
            // BIT 6,(HL)
            case 0x76: OPCODE_BIT_REGPTR_8_BIT(6, HL);
            // BIT 6,A
            case 0x77: OPCODE_BIT_REG_8_BIT(6, A);
            // BIT 7,B
            case 0x78: OPCODE_BIT_REG_8_BIT(7, B);
            // BIT 7,C
            case 0x79: OPCODE_BIT_REG_8_BIT(7, C);
            // BIT 7,D
            case 0x7A: OPCODE_BIT_REG_8_BIT(7, D);
            // BIT 7,E
            case 0x7B: OPCODE_BIT_REG_8_BIT(7, E);
            // BIT 7,H
            case 0x7C: OPCODE_BIT_REG_8_BIT(7, H);
            // BIT 7,L
            case 0x7D: OPCODE_BIT_REG_8_BIT(7, L);
            // BIT 7,(HL)
            case 0x7E: OPCODE_BIT_REGPTR_8_BIT(7, HL);
            // BIT 7,A
            case 0x7F: OPCODE_BIT_REG_8_BIT(7, A);
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
            // SWAP A
            case 0x37: OPCODE_SWAP_REG_8_BIT(A);
            // SWAP B
            case 0x30: OPCODE_SWAP_REG_8_BIT(B);
            // SWAP C
            case 0x31: OPCODE_SWAP_REG_8_BIT(C);
            // SWAP D
            case 0x32: OPCODE_SWAP_REG_8_BIT(D);
            // SWAP E
            case 0x33: OPCODE_SWAP_REG_8_BIT(E);
            // SWAP H
            case 0x34: OPCODE_SWAP_REG_8_BIT(H);
            // SWAP L
            case 0x35: OPCODE_SWAP_REG_8_BIT(L);
            // SWAP (HL)
            case 0x36: OPCODE_SWAP_REGPTR_8_BIT(HL);
            // RES 0, B
            case 0x80: OPCODE_RES_REG_8_BIT(0, B);
            // RES 0, C
            case 0x81: OPCODE_RES_REG_8_BIT(0, C);
            // RES 0, D
            case 0x82: OPCODE_RES_REG_8_BIT(0, D);
            // RES 0, E
            case 0x83: OPCODE_RES_REG_8_BIT(0, E);
            // RES 0, H
            case 0x84: OPCODE_RES_REG_8_BIT(0, H);
            // RES 0, L
            case 0x85: OPCODE_RES_REG_8_BIT(0, L);
            // RES 0, (HL)
            case 0x86: OPCODE_RES_REGPTR_8_BIT(0, HL);
            // RES 0, A
            case 0x87: OPCODE_RES_REG_8_BIT(0, A);
            // RES 1, B
            case 0x88: OPCODE_RES_REG_8_BIT(1, B);
            // RES 1, C
            case 0x89: OPCODE_RES_REG_8_BIT(1, C);
            // RES 1, D
            case 0x8A: OPCODE_RES_REG_8_BIT(1, D);
            // RES 1, E
            case 0x8B: OPCODE_RES_REG_8_BIT(1, E);
            // RES 1, H
            case 0x8C: OPCODE_RES_REG_8_BIT(1, H);
            // RES 1, L
            case 0x8D: OPCODE_RES_REG_8_BIT(1, L);
            // RES 1, (HL)
            case 0x8E: OPCODE_RES_REGPTR_8_BIT(1, HL);
            // RES 1, A
            case 0x8F: OPCODE_RES_REG_8_BIT(1, A);
            // RES 2, B
            case 0x90: OPCODE_RES_REG_8_BIT(2, B);
            // RES 2, C
            case 0x91: OPCODE_RES_REG_8_BIT(2, C);
            // RES 2, D
            case 0x92: OPCODE_RES_REG_8_BIT(2, D);
            // RES 2, E
            case 0x93: OPCODE_RES_REG_8_BIT(2, E);
            // RES 2, H
            case 0x94: OPCODE_RES_REG_8_BIT(2, H);
            // RES 2, L
            case 0x95: OPCODE_RES_REG_8_BIT(2, L);
            // RES 2, (HL)
            case 0x96: OPCODE_RES_REGPTR_8_BIT(2, HL);
            // RES 2, A
            case 0x97: OPCODE_RES_REG_8_BIT(2, A);
            // RES 3, B
            case 0x98: OPCODE_RES_REG_8_BIT(3, B);
            // RES 3, C
            case 0x99: OPCODE_RES_REG_8_BIT(3, C);
            // RES 3, D
            case 0x9A: OPCODE_RES_REG_8_BIT(3, D);
            // RES 3, E
            case 0x9B: OPCODE_RES_REG_8_BIT(3, E);
            // RES 3, H
            case 0x9C: OPCODE_RES_REG_8_BIT(3, H);
            // RES 3, L
            case 0x9D: OPCODE_RES_REG_8_BIT(3, L);
            // RES 3, (HL)
            case 0x9E: OPCODE_RES_REGPTR_8_BIT(3, HL);
            // RES 3, A
            case 0x9F: OPCODE_RES_REG_8_BIT(3, A);
            // RES 4, B
            case 0xA0: OPCODE_RES_REG_8_BIT(4, B);
            // RES 4, C
            case 0xA1: OPCODE_RES_REG_8_BIT(4, C);
            // RES 4, D
            case 0xA2: OPCODE_RES_REG_8_BIT(4, D);
            // RES 4, E
            case 0xA3: OPCODE_RES_REG_8_BIT(4, E);
            // RES 4, H
            case 0xA4: OPCODE_RES_REG_8_BIT(4, H);
            // RES 4, L
            case 0xA5: OPCODE_RES_REG_8_BIT(4, L);
            // RES 4, (HL)
            case 0xA6: OPCODE_RES_REGPTR_8_BIT(4, HL);
            // RES 4, A
            case 0xA7: OPCODE_RES_REG_8_BIT(4, A);
            // RES 5, B
            case 0xA8: OPCODE_RES_REG_8_BIT(5, B);
            // RES 5, C
            case 0xA9: OPCODE_RES_REG_8_BIT(5, C);
            // RES 5, D
            case 0xAA: OPCODE_RES_REG_8_BIT(5, D);
            // RES 5, E
            case 0xAB: OPCODE_RES_REG_8_BIT(5, E);
            // RES 5, H
            case 0xAC: OPCODE_RES_REG_8_BIT(5, H);
            // RES 5, L
            case 0xAD: OPCODE_RES_REG_8_BIT(5, L);
            // RES 5, (HL)
            case 0xAE: OPCODE_RES_REGPTR_8_BIT(5, HL);
            // RES 5, A
            case 0xAF: OPCODE_RES_REG_8_BIT(5, A);
            // RES 6, B
            case 0xB0: OPCODE_RES_REG_8_BIT(6, B);
            // RES 6, C
            case 0xB1: OPCODE_RES_REG_8_BIT(6, C);
            // RES 6, D
            case 0xB2: OPCODE_RES_REG_8_BIT(6, D);
            // RES 6, E
            case 0xB3: OPCODE_RES_REG_8_BIT(6, E);
            // RES 6, H
            case 0xB4: OPCODE_RES_REG_8_BIT(6, H);
            // RES 6, L
            case 0xB5: OPCODE_RES_REG_8_BIT(6, L);
            // RES 6, (HL)
            case 0xB6: OPCODE_RES_REGPTR_8_BIT(6, HL);
            // RES 6, A
            case 0xB7: OPCODE_RES_REG_8_BIT(6, A);
            // RES 7, B
            case 0xB8: OPCODE_RES_REG_8_BIT(7, B);
            // RES 7, C
            case 0xB9: OPCODE_RES_REG_8_BIT(7, C);
            // RES 7, D
            case 0xBA: OPCODE_RES_REG_8_BIT(7, D);
            // RES 7, E
            case 0xBB: OPCODE_RES_REG_8_BIT(7, E);
            // RES 7, H
            case 0xBC: OPCODE_RES_REG_8_BIT(7, H);
            // RES 7, L
            case 0xBD: OPCODE_RES_REG_8_BIT(7, L);
            // RES 7, (HL)
            case 0xBE: OPCODE_RES_REGPTR_8_BIT(7, HL);
            // RES 7, A
            case 0xBF: OPCODE_RES_REG_8_BIT(7, A);
            // SET 0, B
            case 0xC0: OPCODE_SET_REG_8_BIT(0, B);
            // SET 0, C
            case 0xC1: OPCODE_SET_REG_8_BIT(0, C);
            // SET 0, D
            case 0xC2: OPCODE_SET_REG_8_BIT(0, D);
            // SET 0, E
            case 0xC3: OPCODE_SET_REG_8_BIT(0, E);
            // SET 0, H
            case 0xC4: OPCODE_SET_REG_8_BIT(0, H);
            // SET 0, L
            case 0xC5: OPCODE_SET_REG_8_BIT(0, L);
            // SET 0, (HL)
            case 0xC6: OPCODE_SET_REGPTR_8_BIT(0, HL);
            // SET 0, A
            case 0xC7: OPCODE_SET_REG_8_BIT(0, A);
            // SET 1, B
            case 0xC8: OPCODE_SET_REG_8_BIT(1, B);
            // SET 1, C
            case 0xC9: OPCODE_SET_REG_8_BIT(1, C);
            // SET 1, D
            case 0xCA: OPCODE_SET_REG_8_BIT(1, D);
            // SET 1, E
            case 0xCB: OPCODE_SET_REG_8_BIT(1, E);
            // SET 1, H
            case 0xCC: OPCODE_SET_REG_8_BIT(1, H);
            // SET 1, L
            case 0xCD: OPCODE_SET_REG_8_BIT(1, L);
            // SET 1, (HL)
            case 0xCE: OPCODE_SET_REGPTR_8_BIT(1, HL);
            // SET 1, A
            case 0xCF: OPCODE_SET_REG_8_BIT(1, A);
            // SET 2, B
            case 0xD0: OPCODE_SET_REG_8_BIT(2, B);
            // SET 2, C
            case 0xD1: OPCODE_SET_REG_8_BIT(2, C);
            // SET 2, D
            case 0xD2: OPCODE_SET_REG_8_BIT(2, D);
            // SET 2, E
            case 0xD3: OPCODE_SET_REG_8_BIT(2, E);
            // SET 2, H
            case 0xD4: OPCODE_SET_REG_8_BIT(2, H);
            // SET 2, L
            case 0xD5: OPCODE_SET_REG_8_BIT(2, L);
            // SET 2, (HL)
            case 0xD6: OPCODE_SET_REGPTR_8_BIT(2, HL);
            // SET 2, A
            case 0xD7: OPCODE_SET_REG_8_BIT(2, A);
            // SET 3, B
            case 0xD8: OPCODE_SET_REG_8_BIT(3, B);
            // SET 3, C
            case 0xD9: OPCODE_SET_REG_8_BIT(3, C);
            // SET 3, D
            case 0xDA: OPCODE_SET_REG_8_BIT(3, D);
            // SET 3, E
            case 0xDB: OPCODE_SET_REG_8_BIT(3, E);
            // SET 3, H
            case 0xDC: OPCODE_SET_REG_8_BIT(3, H);
            // SET 3, L
            case 0xDD: OPCODE_SET_REG_8_BIT(3, L);
            // SET 3, (HL)
            case 0xDE: OPCODE_SET_REGPTR_8_BIT(3, HL);
            // SET 3, A
            case 0xDF: OPCODE_SET_REG_8_BIT(3, A);
            // SET 4, B
            case 0xE0: OPCODE_SET_REG_8_BIT(4, B);
            // SET 4, C
            case 0xE1: OPCODE_SET_REG_8_BIT(4, C);
            // SET 4, D
            case 0xE2: OPCODE_SET_REG_8_BIT(4, D);
            // SET 4, E
            case 0xE3: OPCODE_SET_REG_8_BIT(4, E);
            // SET 4, H
            case 0xE4: OPCODE_SET_REG_8_BIT(4, H);
            // SET 4, L
            case 0xE5: OPCODE_SET_REG_8_BIT(4, L);
            // SET 4, (HL)
            case 0xE6: OPCODE_SET_REGPTR_8_BIT(4, HL);
            // SET 4, A
            case 0xE7: OPCODE_SET_REG_8_BIT(4, A);
            // SET 5, B
            case 0xE8: OPCODE_SET_REG_8_BIT(5, B);
            // SET 5, C
            case 0xE9: OPCODE_SET_REG_8_BIT(5, C);
            // SET 5, D
            case 0xEA: OPCODE_SET_REG_8_BIT(5, D);
            // SET 5, E
            case 0xEB: OPCODE_SET_REG_8_BIT(5, E);
            // SET 5, H
            case 0xEC: OPCODE_SET_REG_8_BIT(5, H);
            // SET 5, L
            case 0xED: OPCODE_SET_REG_8_BIT(5, L);
            // SET 5, (HL)
            case 0xEE: OPCODE_SET_REGPTR_8_BIT(5, HL);
            // SET 5, A
            case 0xEF: OPCODE_SET_REG_8_BIT(5, A);
            // SET 6, B
            case 0xF0: OPCODE_SET_REG_8_BIT(6, B);
            // SET 6, C
            case 0xF1: OPCODE_SET_REG_8_BIT(6, C);
            // SET 6, D
            case 0xF2: OPCODE_SET_REG_8_BIT(6, D);
            // SET 6, E
            case 0xF3: OPCODE_SET_REG_8_BIT(6, E);
            // SET 6, H
            case 0xF4: OPCODE_SET_REG_8_BIT(6, H);
            // SET 6, L
            case 0xF5: OPCODE_SET_REG_8_BIT(6, L);
            // SET 6, (HL)
            case 0xF6: OPCODE_SET_REGPTR_8_BIT(6, HL);
            // SET 6, A
            case 0xF7: OPCODE_SET_REG_8_BIT(6, A);
            // SET 7, B
            case 0xF8: OPCODE_SET_REG_8_BIT(7, B);
            // SET 7, C
            case 0xF9: OPCODE_SET_REG_8_BIT(7, C);
            // SET 7, D
            case 0xFA: OPCODE_SET_REG_8_BIT(7, D);
            // SET 7, E
            case 0xFB: OPCODE_SET_REG_8_BIT(7, E);
            // SET 7, H
            case 0xFC: OPCODE_SET_REG_8_BIT(7, H);
            // SET 7, L
            case 0xFD: OPCODE_SET_REG_8_BIT(7, L);
            // SET 7, (HL)
            case 0xFE: OPCODE_SET_REGPTR_8_BIT(7, HL);
            // SET 7, A
            case 0xFF: OPCODE_SET_REG_8_BIT(7, A);
            // SLA A
            case 0x27: OPCODE_SLA_REG_8_BIT(A);
            // SLA B
            case 0x20: OPCODE_SLA_REG_8_BIT(B);
            // SLA C
            case 0x21: OPCODE_SLA_REG_8_BIT(C);
            // SLA D
            case 0x22: OPCODE_SLA_REG_8_BIT(D);
            // SLA E
            case 0x23: OPCODE_SLA_REG_8_BIT(E);
            // SLA H
            case 0x24: OPCODE_SLA_REG_8_BIT(H);
            // SLA L
            case 0x25: OPCODE_SLA_REG_8_BIT(L);
            // SLA (HL)
            case 0x26: OPCODE_SLA_REGPTR_8_BIT(HL);
            // SRA A
            case 0x2F: OPCODE_SRA_REG_8_BIT(A);
            // SRA B
            case 0x28: OPCODE_SRA_REG_8_BIT(B);
            // SRA C
            case 0x29: OPCODE_SRA_REG_8_BIT(C);
            // SRA D
            case 0x2A: OPCODE_SRA_REG_8_BIT(D);
            // SRA E
            case 0x2B: OPCODE_SRA_REG_8_BIT(E);
            // SRA H
            case 0x2C: OPCODE_SRA_REG_8_BIT(H);
            // SRA L
            case 0x2D: OPCODE_SRA_REG_8_BIT(L);
            // SRA (HL)
            case 0x2E: OPCODE_SRA_REGPTR_8_BIT(HL);
            // SRL A
            case 0x3F: OPCODE_SRL_REG_8_BIT(A);
            // SRL B
            case 0x38: OPCODE_SRL_REG_8_BIT(B);
            // SRL C
            case 0x39: OPCODE_SRL_REG_8_BIT(C);
            // SRL D
            case 0x3A: OPCODE_SRL_REG_8_BIT(D);
            // SRL E
            case 0x3B: OPCODE_SRL_REG_8_BIT(E);
            // SRL H
            case 0x3C: OPCODE_SRL_REG_8_BIT(H);
            // SRL L
            case 0x3D: OPCODE_SRL_REG_8_BIT(L);
            // SRL (HL)
            case 0x3E: OPCODE_SRL_REGPTR_8_BIT(HL);

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





















