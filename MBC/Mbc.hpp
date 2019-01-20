
#ifndef _MBC_HPP_
#define _MBC_HPP_

#include <cstdint>
#include "Util/LogUtil.hpp"

class Mbc {
protected:
    int romBankNumber = 1;
    int ramBankNumber;

public:
    int getRomBankNumber() {
        return romBankNumber;
    }

    int getRamBankNumber() {
        return ramBankNumber;
    }

    virtual bool write8(uint16_t, uint8_t) = 0;
    virtual ~Mbc() {};
};

#endif