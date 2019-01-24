
#include "MBC/Mbc1.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

bool Mbc1::write8(uint16_t address, uint8_t value) {
    if (address >= 0x0000 && address <= 0x1FFF) {
        ramEnabled = lowNibbleOf(value) == 0xA;
        if (isRamEnabled()) {
            //cout << "RAM is enabled" << endl;
        }
        return true;

    } else if (address >= 0x2000 && address <= 0x3FFF) {
        romBankNumber = (romBankNumber & 0b1100000) | (value & 0b00011111);
        if (romBankNumber == 0) romBankNumber = 1;
        return true;

    } else if (address >= 0x4000 && address <= 0x5FFF) {
        if (romMode) {
            romBankNumber = (romBankNumber & 0b0011111) | ((value & 0b11) << 5);
            if (romBankNumber == 0) romBankNumber = 1;
        } else {
            ramBankNumber = value & 0b11; 
        }

        return true;

    } else if (address >= 0x6000 && address <= 0x7FFF) {
        if (value == 0x00) {
            romMode = true;
        }
        else if (value == 0x01) {
            romMode = false;
        }
        else {
            TRACE_MBC("Writing illegal value into MBC1 bank mode selection!" << endl);
        }
        return true;

    }
    return false;
}