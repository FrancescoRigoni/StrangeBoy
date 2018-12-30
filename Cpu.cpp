
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

#define OPCODE_DEC_REG_8_BIT(REG) { \
    TRACE_CPU("DEC " << #REG); \
    setReg##REG(reg##REG()-1); \
    if (reg##REG() == 0) setFlag(FLAG_ZERO); \
    if (lowNibbleOf(reg##REG()) == 0xF) setFlag(FLAG_HALF_CARRY); \
    setFlag(FLAG_SUBTRACT); \
    ADD_CYCLES(4); \
    break; \
}

#define OPCODE_LD_NN_N_8_BIT(REG) { \
    uint8_t arg = imm8(); \
    TRACE_CPU("LD " << #REG << "," << cout8Hex(arg)); \
    setReg##REG(arg); \
    ADD_CYCLES(8); \
    break; \
}

void Cpu::cycle() {
    uint8_t opcode = memory->read8(regPC++);
    TRACE_CPU(cout16Hex(regPC-1) << "  :  " << cout8Hex(opcode) << "  :  ");

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
        // LD B,$n
        case 0x06: OPCODE_LD_NN_N_8_BIT(B);
        // LD D,$n
        case 0x16: OPCODE_LD_NN_N_8_BIT(D);
        // LD L, $n
        case 0x2E: OPCODE_LD_NN_N_8_BIT(L);
        // LD C, $n
        case 0x0E: OPCODE_LD_NN_N_8_BIT(C);
        // LD E, $n
        case 0x1E: OPCODE_LD_NN_N_8_BIT(E);
  
        case 0x18:
        // JR n
        TRACE_CPU("JR " << cout8Signed(imm8()));
        break;

        case 0x3E:
        // LD A, #
        TRACE_CPU("LD A," << cout8Hex(imm8()));
        break;

        case 0x22:
        // LD (HL+), A
        TRACE_CPU("LD (HL+), A");
        break;

        case 0x04:
        // INC B
        TRACE_CPU("INC B");
        break;

        case 0x24:
        // INC H
        TRACE_CPU("INC H");
        break;

        case 0x23:
        // INC HL
        TRACE_CPU("INC HL");
        break;

        case 0x31:
        // LD SP,$nnnn
        TRACE_CPU("LD SP," << cout16Hex(imm16()));
        break;

        case 0xAF:
        // XOR A
        TRACE_CPU("XOR A");
        break;

        case 0x4f:
        // LD C, A
        TRACE_CPU("LD C, A");
        break;

        case 0x7C:
        // LD A, H
        TRACE_CPU("LD A, H");
        break;

        case 0x21:
        // LD HL, $nn
        TRACE_CPU("LD HL," << cout16Hex(imm16()));
        break;

        case 0x32:
        // LD (HL-),A
        TRACE_CPU("LD (HL-),A");
        break;

        case 0xCB:
        opcode = memory->read8(regPC++);
        TRACE_CPU(cout8Hex(opcode) << "  :  ");
        switch (opcode) {
            case 0x7C:
            // BIT 7,H
            TRACE_CPU("BIT 7,H");
            break;

            case 0x11:
            // RL C
            TRACE_CPU("RL C");
        }
        break;

        case 0xE0:
        // LD ($FF00+C),A
        TRACE_CPU("LD ($FF00+" << cout8Hex(imm8()) << "),A");
        break;

        case 0xE2:
        // LD ($FF00+C),A
        TRACE_CPU("LD ($FF00+C),A");
        break;

        case 0xF0:
        // LD A, ($FF00+$n)
        TRACE_CPU("LD A, ($FF00+" << cout8Hex(imm8()) << ")");
        break;

        case 0xEA:
        // LD ($nn), A
        TRACE_CPU("LD (" << cout16Hex(imm16()) << "),A");
        break;

        case 0x0C:
        // INC C
        TRACE_CPU("INC C");
        break;

        case 0x20:
        // JR NZ, nn
        TRACE_CPU("JR NZ," << cout8Signed(imm8()));
        break;

        case 0x11:
        // LD DE,nn
        TRACE_CPU("LD DE," << cout16Hex(imm16()));
        break;

        case 0x13:
        // INC DE
        TRACE_CPU("INC DE");
        break;

        case 0x77:
        // LD (HL), A
        TRACE_CPU("LD (HL), A");
        break;

        case 0x7B:
        // LD A, E
        TRACE_CPU("LD A, E");
        break;

        case 0x1A:
        // LD A, (DE)
        TRACE_CPU("LD A, (DE)");
        break;

        case 0xCD:
        // CALL nn
        TRACE_CPU("CALL " << cout16Hex(imm16()));
        break;

        case 0xFE:
        // CP n
        TRACE_CPU("CP " << cout8Hex(imm8()));
        break;

        case 0x28:
        // JR Z, $n
        TRACE_CPU("JR Z," << cout8Hex(imm8()));
        break;

        case 0x57:
        // LD D, A
        TRACE_CPU("LD D, A");
        break;

        case 0x67:
        // LD H, A
        TRACE_CPU("LD H, A");
        break;

        case 0x90:
        // SUB B
        TRACE_CPU("SUB B");
        break;

        case 0xC5:
        // PUSH BC
        TRACE_CPU("PUSH BC");
        break;

        case 0xC1:
        // POP BC
        TRACE_CPU("POP BC");
        break;

        case 0xC9:
        // RET
        TRACE_CPU("RET");
        break;

        case 0x17:
        // RLA
        TRACE_CPU("RLA");
        break;

        default:
        TRACE_CPU("Unimplemented!!!");
        break;
    }

    TRACE_CPU(endl)
}
























