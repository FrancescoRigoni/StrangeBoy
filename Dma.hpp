
#ifndef __DMA_H__
#define __DMA_H__

#include <cstdint>
#include "Memory.hpp"
#include "LogUtil.hpp"
#include "ByteUtil.hpp"
#include "IoDevice.hpp"

#define DMA 0xFF46

/**
Typical DMA wait routine
ld      a, $28            // 8 cycles
.loop:
  dec     a               // 4 cycles
  jr      nz, .loop       // 8/12 cycles
*/
#define DMA_TRANSFER_CPU_CYCLES (8 + (0x28*4) + (0x27*8) + 12)

class Dma : public IoDevice {
private:
    Memory *memory;

    int cpuCyclesLeftForTransfer = 0;
    uint16_t currentSourceAddress = 0;
    uint16_t currentDestinationAddress = 0;

public:
    Dma(Memory *memory);

    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    void cycle(int cycles);
    bool isTransferInProgress();
};

#endif