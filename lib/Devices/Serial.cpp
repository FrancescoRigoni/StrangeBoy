#include "Devices/Serial.hpp"

Serial::Serial(InterruptFlags *interruptFlags) {
    this->interruptFlags = interruptFlags;
}

void Serial::update() {
    if (triggerTransfer) {
        triggerTransfer = false;
        delay = 10;
    } else if (delay > 0) {
        delay--;
        if (delay == 0) {
            interruptFlags->interruptSerial();
            resetBit(7, &data);
        }
    }
}

void Serial::write8(uint16_t address, uint8_t value) {
    switch (address) {
        case SERIAL_TX_DATA: {
            data = value;
            triggerTransfer = isBitSet(data, 7);
            break;
        }
        case SERIAL_IO_CTRL: {
            control = value; 
            break;
        }
    }
}

uint8_t Serial::read8(uint16_t address) {
    switch (address) {
        case SERIAL_TX_DATA: return data;
        case SERIAL_IO_CTRL: return control;
    }
    return 0;
}
