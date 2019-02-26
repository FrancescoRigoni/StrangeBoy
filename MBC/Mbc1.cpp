
#include "MBC/Mbc1.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

bool Mbc1::write8(uint16_t address, uint8_t value) {
    if (address >= 0x0000 && address <= 0x1FFF) {
        TRACE_MBC("Writing ram enable" << endl);
        ramEnabled = lowNibbleOf(value) == 0xA;
        return true;

    } else if (address >= 0x2000 && address <= 0x3FFF) {
        uint8_t lowBitsOfRomBankNumber = value & 0b0011111;
        if (lowBitsOfRomBankNumber == 0) lowBitsOfRomBankNumber = 1;
        romBankNumber = romBankNumber & 0b1100000;
        romBankNumber |= lowBitsOfRomBankNumber;

        TRACE_MBC("Writing romBankNumber low " << cout8Hex(romBankNumber) << endl);
        return true;

    } else if (address >= 0x4000 && address <= 0x5FFF) {
        if (romMode) {
            romBankNumber &= 0b11111;
            romBankNumber |= ((romBankNumber & 0b1100000) << 0);
            if (romBankNumber == 0x20 || 
                romBankNumber == 0x40 || 
                romBankNumber == 0x60) romBankNumber += 1;

            TRACE_MBC("Writing romBankNumber high " << cout8Hex(romBankNumber) << endl);
        } else {
            ramBankNumber = value & 0b11;
            TRACE_MBC("Writing ramBankNumber " << cout8Hex(ramBankNumber) << endl);
        }

        return true;

    } else if (address >= 0x6000 && address <= 0x7FFF) {
        if (value == 0x00) {
            romMode = true;
            ramBankNumber = 0;
            TRACE_MBC("ROM mode true " << endl);
        }
        else if (value == 0x01) {
            romMode = false;
            TRACE_MBC("ROM mode false " << endl);
            romBankNumber &= 0b11111;
        }
        else {
            TRACE_MBC("Writing illegal value into MBC1 bank mode selection! " << cout8Hex(value) << endl);
        }
        return true;
    }
    return false;
}