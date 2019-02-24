
#include "Devices/InterruptFlags.hpp"

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

void InterruptFlags::interruptJoypad() {
    if (isBitSet(enableMask, IE_BIT_JOYPAD)) {
        setBit(IF_BIT_JOYPAD, &flags);
    }
}


bool InterruptFlags::acknowledgeJoypadInterrupt() {
    if (isBitSet(flags, IF_BIT_JOYPAD) && 
        isBitSet(enableMask, IE_BIT_JOYPAD)) {
        resetBit(IF_BIT_JOYPAD, &flags);
        return true;
    }
    return false;
}

void InterruptFlags::interruptTimer() {
    if (isBitSet(enableMask, IE_BIT_TIMER)) {
        setBit(IF_BIT_TIMER, &flags);
    }
}


bool InterruptFlags::acknowledgeTimerInterrupt() {
    if (isBitSet(flags, IF_BIT_TIMER) && 
        isBitSet(enableMask, IE_BIT_TIMER)) {
        resetBit(IF_BIT_TIMER, &flags);
        return true;
    }
    return false;
}