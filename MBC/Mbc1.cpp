
#include "MBC/Mbc1.hpp"
#include "Util/LogUtil.hpp"

bool Mbc1::write8(uint16_t address, uint8_t value) {
    if (address >= 0x0000 && address <= 0x1FFF) {
        TRACE_MBC("Access to unhandled MBC1 write area" << endl);
    } else if (address >= 0x2000 && address <= 0x3FFF) {
        romBankNumber = value & 0b00011111;
        if (romBankNumber == 0) romBankNumber = 1;
        return true;
    } else if (address >= 0x4000 && address <= 0x5FFF) {
        TRACE_MBC("Access to unhandled MBC1 write area" << endl);
    } else if (address >= 0x6000 && address <= 0x7FFF) {
        TRACE_MBC("Access to unhandled MBC1 write area" << endl);
    }
    return false;
}