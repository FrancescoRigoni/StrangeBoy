
#include "Joypad.hpp"

void Joypad::write8(uint16_t address, uint8_t value) {
    TRACE_JOYPAD(endl << "Joypad write " << cout8Hex(value) << endl);
    pSelect = value;
}

uint8_t Joypad::read8(uint16_t address) {
    TRACE_JOYPAD(endl << "Joypad read " << endl);
    uint8_t out = 0;
    if (isBitClear(pSelect, JOYPAD_P14_BIT)) {
        if (buttonRight) out |= (1 << JOYPAD_P10_BIT);
        if (buttonLeft) out |= (1 << JOYPAD_P11_BIT);
        if (buttonUp) out |= (1 << JOYPAD_P12_BIT);
        if (buttonDown) out |= (1 << JOYPAD_P13_BIT);

    } else if (isBitClear(pSelect, JOYPAD_P15_BIT)) {
        if (buttonA) out |= (1 << JOYPAD_P10_BIT);
        if (buttonB) out |= (1 << JOYPAD_P11_BIT);
        if (buttonSelect) out |= (1 << JOYPAD_P12_BIT);
        if (buttonStart) out |= (1 << JOYPAD_P13_BIT);
    }

    return ~out;
}