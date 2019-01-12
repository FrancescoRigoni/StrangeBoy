#ifndef __INTERRUPT_FLAGS_H__
#define __INTERRUPT_FLAGS_H__

#include <cstdint>
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"

#define IF                    0xFF0F
#define INTERRUPTS_ENABLE_REG 0xFFFF

#define IE_BIT_VBLANK   0
#define IE_BIT_LCDC     1
#define IE_BIT_TIMER    2
#define IE_BIT_SERIAL   3
#define IE_BIT_JOYP     4

#define IF_BIT_VBLANK   0
#define IF_BIT_LCDC     1

class InterruptFlags : public IoDevice {
private:
    uint8_t flags;
    uint8_t enableMask;

public:
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    void interruptVBlank();
    bool acknowledgeVBlankInterrupt();

    void interruptLCDC();
    bool acknowledgeLCDCInterrupt();
};

#endif