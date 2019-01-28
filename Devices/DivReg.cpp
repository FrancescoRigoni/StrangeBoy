#include "Devices/DivReg.hpp"
#include <iostream>
#include "Util/LogUtil.hpp"

#define DIV_REG_INCREMENTS_PER_MILLISECOND (16384.0 / 1000.0)

void DivReg::write8(uint16_t address, uint8_t value) {
    value = 0;
}

uint8_t DivReg::read8(uint16_t address) {
    return value;
}

void DivReg::increment() {
    if (lastIncrement == 0) {
        lastIncrement = chrono::duration_cast<chrono::milliseconds> 
            (chrono::system_clock::now().time_since_epoch()).count();
        return;
    }

    long now = chrono::duration_cast<chrono::milliseconds>
        (chrono::system_clock::now().time_since_epoch()).count();
    long msSinceLastIncrement = now - lastIncrement;
    int increments = DIV_REG_INCREMENTS_PER_MILLISECOND * msSinceLastIncrement;

    value+=increments;

    lastIncrement = chrono::duration_cast<chrono::milliseconds> 
            (chrono::system_clock::now().time_since_epoch()).count();
}