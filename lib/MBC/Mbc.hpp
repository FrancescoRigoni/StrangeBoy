
#ifndef _MBC_HPP_
#define _MBC_HPP_

#include <cstdint>
#include "Util/LogUtil.hpp"

class Mbc {
protected:
    int romBankNumber = 1;
    int ramBankNumber = 0;
    bool ramEnabled = false;

public:
    int getRomBankNumber() {
        return romBankNumber;
    }

    int getRamBankNumber() {
        return ramBankNumber;
    }

    bool isRamEnabled() {
        return ramEnabled;
    }

    virtual bool write8(uint16_t, uint8_t) = 0;
    virtual ~Mbc() {};
};

#endif