
#include "Dma.hpp"
#include "Memory.hpp"

Dma::Dma(Memory *memory) {
    this->memory = memory;
}

void Dma::write8(uint8_t value) {
    TRACE_DMA("Dma write" << endl);
    cpuCyclesLeftForTransfer = DMA_TRANSFER_CPU_CYCLES;
    currentSourceAddress = ((uint16_t)value) << 8;
    currentDestinationAddress = OAM_RAM_START;
}

uint8_t Dma::read8(uint16_t address) {
    TRACE_DMA("Dma read" << endl);
    return 0;
}

void Dma::cycle(int cycles) {
    if (cpuCyclesLeftForTransfer <= 0) return;

    for (int availableCycles = cycles; availableCycles > 0; availableCycles--) {
        memory->write8(currentDestinationAddress++, memory->read8(currentSourceAddress++, false), false);
        cpuCyclesLeftForTransfer--;

        if (currentDestinationAddress == (OAM_RAM_START + OAM_RAM_SIZE)) {
            cpuCyclesLeftForTransfer = 0;
            currentSourceAddress = 0;
            currentDestinationAddress = 0;
            break;
        }
    }
}

bool Dma::isTransferInProgress() {
    return cpuCyclesLeftForTransfer > 0;
}
