
#ifndef _MBCDUMMY_HPP_
#define _MBCDUMMY_HPP_

#include <cstdint>
#include "MBC/Mbc.hpp"

class MbcDummy : public Mbc {
public:
    virtual bool write8(uint16_t address, uint8_t value) override {
        return (address >= 0x2000 && address <= 0x7FFF);
    }
};

#endif