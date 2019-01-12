
#include "Joypad.hpp"

void Joypad::write8(uint16_t address, uint8_t value) {
    TRACE_JOYPAD(endl << "Joypad write " << cout8Hex(value) << endl);
    pSelect = value;
}

uint8_t Joypad::read8(uint16_t address) {
    TRACE_JOYPAD(endl << "Joypad read " << endl);
    uint8_t out = 0;

    if (isBitClear(pSelect, JOYPAD_P14_BIT)) {
        if (buttonRight.load()) out |= (1 << JOYPAD_P10_BIT);
        if (buttonLeft.load()) out |= (1 << JOYPAD_P11_BIT);
        if (buttonUp.load()) out |= (1 << JOYPAD_P12_BIT);
        if (buttonDown.load()) out |= (1 << JOYPAD_P13_BIT);

    } else if (isBitClear(pSelect, JOYPAD_P15_BIT)) {
        if (buttonA.load()) out |= (1 << JOYPAD_P10_BIT);
        if (buttonB.load()) out |= (1 << JOYPAD_P11_BIT);
        if (buttonSelect.load()) out |= (1 << JOYPAD_P12_BIT);
        if (buttonStart.load()) out |= (1 << JOYPAD_P13_BIT);
    }

    return ~out;
}