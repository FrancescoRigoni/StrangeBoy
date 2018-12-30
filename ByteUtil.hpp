
#include <cstdint>

#ifndef __BYTEUTIL_H__
#define __BYTEUTIL_H__

inline uint8_t lowNibbleOf(uint8_t byte) {
    return byte & 0xF;
}

inline uint8_t highNibbleOf(uint8_t byte) {
    return (byte & 0xF0) >> 4;
}

inline uint8_t msbOf(uint16_t dbyte) {
    return (uint8_t)((dbyte & 0xFF00) >> 8);
}

inline uint8_t lsbOf(uint16_t dbyte) {
    return (uint8_t)(dbyte & 0xFF);
}

inline void msbTo(uint16_t *dbyte, uint8_t msb) {
    *dbyte = (*dbyte & 0x00FF) | ((uint16_t)msb << 8);
}

inline void lsbTo(uint16_t *dbyte, uint8_t lsb) {
    *dbyte = (*dbyte & 0xFF00) | (uint16_t)lsb;
}

inline uint16_t littleEndianCombine(uint8_t lsb, uint8_t msb) {
    return (uint16_t)lsb | ((uint16_t)msb << 8);
}

inline bool isBitSet(uint8_t in, uint8_t bit) {
    uint8_t mask = 1 << bit;
    return in & mask;
}

inline bool isBitClear(uint8_t in, uint8_t bit) {
    return !isBitSet(in, bit);
}

#endif