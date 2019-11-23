
#ifndef __JOYPAD_H__
#define __JOYPAD_H__

#include <cstdint>
#include <atomic>

#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"

#define P1 0xFF00

#define JOYPAD_P15_BIT 5
#define JOYPAD_P14_BIT 4
#define JOYPAD_P13_BIT 3
#define JOYPAD_P12_BIT 2
#define JOYPAD_P11_BIT 1
#define JOYPAD_P10_BIT 0      

using namespace std;

class Joypad : public IoDevice {
private:
    uint8_t pSelect;

public:
    atomic<bool> buttonRight{false};
    atomic<bool> buttonLeft{false};
    atomic<bool> buttonUp{false};
    atomic<bool> buttonDown{false};
    atomic<bool> buttonA{false};
    atomic<bool> buttonB{false};
    atomic<bool> buttonSelect{false};
    atomic<bool> buttonStart{false};
    
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);
};

#endif