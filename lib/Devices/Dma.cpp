
#include "Devices/Dma.hpp"
#include "Cpu/Memory.hpp"

Dma::Dma(Memory *memory) {
    this->memory = memory;
}

void Dma::write8(uint16_t address, uint8_t value) {
    TRACE_DMA("Dma write" << endl);
    cpuCyclesLeftForTransfer = DMA_TRANSFER_CPU_CYCLES;
    currentSourceAddress = ((uint16_t)value) << 8;
    currentDestinationAddress = OAM_RAM_START;
}

uint8_t Dma::read8(uint16_t address) {
    TRACE_DMA("Dma read" << endl);
    return 0;
}

void Dma::tick(int cycles) {
    if (cpuCyclesLeftForTransfer <= 0) return;

    for (int availableCycles = cycles; availableCycles > 0; availableCycles--) {
        memory->write8(currentDestinationAddress++, memory->read8(currentSourceAddress++));
        cpuCyclesLeftForTransfer--;

        if (currentDestinationAddress == (OAM_RAM_START + OAM_RAM_SIZE)) {
            cpuCyclesLeftForTransfer = 0;
            currentSourceAddress = 0;
            currentDestinationAddress = 0;
            break;
        }
    }
}
