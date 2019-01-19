
#ifndef _CARTRIDGE_HPP_
#define _CARTRIDGE_HPP_

#include <cstdint>

#define CARTRIDGE_INFO_START_OFFSET 0x134

#define CART_TYPE_ROM_ONLY              0
#define CART_TYPE_ROM_MBC1              1


typedef struct {
    uint8_t gameTitle[16];
    uint8_t colorGameBoy;
    uint8_t licenseeCodeHigh;
    uint8_t licenseeCodeLow;
    uint8_t isGBOrSGB;
    uint8_t cartridgeType;
    uint8_t romSize;
    uint8_t ramSize;
    uint8_t destinationCode;
    uint8_t oldLicenseeCode;
    uint8_t maskRomVersion;
    uint8_t complementCheck;
    uint8_t checksum[2];
} CartridgeInfo;

class Cartridge {
    public:
        void parse(uint8_t*, CartridgeInfo*);

};

#endif