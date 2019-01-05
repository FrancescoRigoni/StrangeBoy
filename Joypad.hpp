
#ifndef __JOYPAD_H__
#define __JOYPAD_H__

#include <cstdint>
#include "LogUtil.hpp"
#include "ByteUtil.hpp"
#include "IoDevice.hpp"

#define P1 0xFF00

#define JOYPAD_P15_BIT 5
#define JOYPAD_P14_BIT 4
#define JOYPAD_P13_BIT 3
#define JOYPAD_P12_BIT 2
#define JOYPAD_P11_BIT 1
#define JOYPAD_P10_BIT 0      

class Joypad : public IoDevice {
private:
    bool buttonRight = false;
    bool buttonLeft = false;
    bool buttonUp = false;
    bool buttonDown = false;
    bool buttonA = false;
    bool buttonB = false;
    bool buttonSelect = false;
    bool buttonStart = false;
   
    uint8_t pSelect;

public:
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);
};

#endif