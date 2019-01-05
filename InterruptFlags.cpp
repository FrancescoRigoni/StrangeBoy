
#include "InterruptFlags.hpp"

void InterruptFlags::write8(uint16_t address, uint8_t value) {
    if (address == IF) this->flags = value;
    else if (address == INTERRUPTS_ENABLE_REG) this->enableMask = value;
    else TRACE_IO(endl << "InterruptFlags : attempt to write to unhandled address " << cout16Hex(address));
}

uint8_t InterruptFlags::read8(uint16_t address) {
    if (address == IF) return flags;
    else if (address == INTERRUPTS_ENABLE_REG) return enableMask;
    else {
        TRACE_IO(endl << "InterruptFlags : attempt to read from unhandled address " << cout16Hex(address));
        return 0;
    }
}

void InterruptFlags::interruptVBlank() {
    if (isBitSet(enableMask, IE_BIT_VBLANK)) {
        setBit(IF_BIT_VBLANK, &flags);
    }
}

bool InterruptFlags::acknowledgeVBlankInterrupt() {
    if (isBitSet(flags, IF_BIT_VBLANK) && 
        isBitSet(enableMask, IE_BIT_VBLANK)) {
        resetBit(IF_BIT_VBLANK, &flags);
        return true;
    }
    return false;
}

void InterruptFlags::interruptLCDC() {
    if (isBitSet(enableMask, IE_BIT_LCDC)) {
        setBit(IF_BIT_LCDC, &flags);
    }
}

bool InterruptFlags::acknowledgeLCDCInterrupt() {
    if (isBitSet(flags, IF_BIT_LCDC) && 
        isBitSet(enableMask, IE_BIT_LCDC)) {
        resetBit(IF_BIT_LCDC, &flags);
        return true;
    }
    return false;
}