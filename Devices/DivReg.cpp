#include "Devices/DivReg.hpp"

#define DIV_REG_FREQUENCY_HZ 16384.0
#define CPU_FREQUENCY_HZ (4*1024*1024)

void DivReg::write8(uint16_t address, uint8_t value) {
    value = 0;
}

uint8_t DivReg::read8(uint16_t address) {
    return value;
}

void DivReg::tick(int cpuCycles) {
    float cpuTimeMilliseconds = ((1.0/CPU_FREQUENCY_HZ)*(float)cpuCycles) * 1000;
    float incrementPerMillisecond = DIV_REG_FREQUENCY_HZ / 1000.0;
    float increments = incrementPerMillisecond * cpuTimeMilliseconds;
    value = value + increments;
    if (value > 255) value = 0;
}