
#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <cstdint>
#include <chrono>
#include "Cpu/Memory.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"
#include "Devices/InterruptFlags.hpp"

using namespace std;

#define SERIAL_TX_DATA      0xFF01
#define SERIAL_IO_CTRL      0xFF02

/*
Note: this class is just a dummy, serial is not implemented.
*/
class Serial : public IoDevice {
private:
    uint8_t data = 0;
    uint8_t control = 0;
    InterruptFlags *interruptFlags;

    int delay = 0;
    bool triggerTransfer = false;

public:
    Serial(InterruptFlags *);
    void update();
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);
};

#endif