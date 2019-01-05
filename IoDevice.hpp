
#ifndef __IO_DEVICE__
#define __IO_DEVICE__

#include <cstdint>

class IoDevice {
public:
    virtual ~IoDevice() {};
    virtual void write8(uint8_t) = 0;
    virtual uint8_t read8(uint16_t) = 0;
};

#endif