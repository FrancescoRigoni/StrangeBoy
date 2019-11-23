
#ifndef __DIVREG_H__
#define __DIVREG_H__

#include <cstdint>
#include "Cpu/Memory.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"

using namespace std;

#define DIV_REG 0xFF04

class DivReg : public IoDevice {
private:
    float value = 0;

public:
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    void tick(int cpuCycles);
};

#endif