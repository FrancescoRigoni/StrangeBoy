
#ifndef __BYTEUTIL_H__
#define __BYTEUTIL_H__

#include <cstdint>

uint8_t msbOf(uint16_t dbyte) {
    return (uint8_t)((dbyte & 0xFF00) >> 8);
}

uint8_t lsbOf(uint16_t dbyte) {
    return (uint8_t)(dbyte & 0xFF);
}

uint16_t littleEndianCombine(uint8_t lsb, uint8_t msb) {
    return (uint16_t)lsb | ((uint16_t)msb << 8);
}

#endif