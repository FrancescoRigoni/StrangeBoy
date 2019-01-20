
#ifndef _MBC1_HPP_
#define _MBC1_HPP_

#include <cstdint>
#include "MBC/Mbc.hpp"

class Mbc1 : public Mbc {
public:
    virtual bool write8(uint16_t, uint8_t) override;
};

#endif