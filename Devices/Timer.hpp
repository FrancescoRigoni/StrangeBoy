
#ifndef _TIMER_H_
#define _TIMER_H_

#include <cstdint>
#include "Cpu/Memory.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"
#include "Devices/InterruptFlags.hpp"

using namespace std;

#define TIMA                0xFF05
#define TMA                 0xFF06
#define TAC                 0xFF07

class Timer : public IoDevice {
private:
    uint8_t modulo = 0;
    uint8_t control = 0;

    InterruptFlags *interruptFlags;
    float floatValue;

    float getFrequencyHz();

public:
    Timer(InterruptFlags *);
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    void tick(int cpuCyles);
};

#endif