
#include "LCDControlAndStat.hpp"
#include "ByteUtil.hpp"

#define BACKGROUND_TILE_SIZE_BYTES 16

void LCDControlAndStat::stateOAMSearch() {
    stat &= 0b11111100;
    stat |= 0b00000010;
}

void LCDControlAndStat::stateDrawLine() {
    stat &= 0b11111100;
    stat |= 0b00000011;
}

void LCDControlAndStat::stateHBlank() {
    stat &= 0b11111100;
}

void LCDControlAndStat::stateVBlank() {
    stat &= 0b11111100;
    stat |= 0b00000001;
}

bool LCDControlAndStat::inOAMSearch() {
    return (stat & 0b00000011) == 0b00000010;
}

bool LCDControlAndStat::inDrawLine() {
    return (stat & 0b00000011) == 0b00000011;
}

bool LCDControlAndStat::inHBlank() {
    return (stat & 0b00000011) == 0b00000000;
}

bool LCDControlAndStat::inVBlank()  {
    return (stat & 0b00000011) == 0b00000001;
}

void LCDControlAndStat::write8(uint16_t address, uint8_t value) {
    if (address == STAT) this->stat = value;
    else if (address == LCDC) this->lcdc = value;
    else if (address == SCY) this->scy = value;
    else if (address == SCX) this->scx = value;
    else if (address == LY) this->ly = value;
    else if (address == LYC) this->lyc = value;
}

uint8_t LCDControlAndStat::read8(uint16_t address) {
    if (address == STAT) return stat;
    else if (address == LCDC) return lcdc;
    else if (address == SCY) return scy;
    else if (address == SCX) return scx;
    else if (address == LY) return ly;
    else if (address == LYC) return lyc;
    else {
        TRACE_IO(endl << "LCDRegs : Access to unhandled address " << cout16Hex(address) << endl);
        return 0;
    }
}

uint16_t LCDControlAndStat::addressForBackgroundTile(uint8_t tileNumber) {
    bool dataSelect = isBitSet(lcdc, LCDC_BG_AND_WIN_TILE_DATA_SELECT_BIT);
    uint16_t baseAddress = dataSelect ? 0x8000 : 0x8800;

    if (dataSelect) {
        // Unsigned offset
        uint16_t uTileOffset = tileNumber * BACKGROUND_TILE_SIZE_BYTES;
        return baseAddress + uTileOffset;
    } else {
        // Signed offset
        int16_t sTileNumber = tileNumber;
        int16_t sTileOffset = sTileNumber * BACKGROUND_TILE_SIZE_BYTES;
        return baseAddress + sTileOffset;
    }
}

uint16_t LCDControlAndStat::addressForBackgroundTilesMap() {
    return isBitSet(lcdc, LCDC_BG_TILE_MAP_DISPLAY_SELECT_BIT) ? 0x9C00 : 0x9800;
}

bool LCDControlAndStat::isScreenOn() {
    return isBitSet(lcdc, LCDC_CONTROL_OP_BIT);
}

bool LCDControlAndStat::isBackgroundOn() {
    return isBitSet(lcdc, LCDC_BG_AND_WIN_DISPLAY_BIT);
}

bool LCDControlAndStat::isWindowOn() {
    return isBitSet(lcdc, LCDC_BG_AND_WIN_DISPLAY_BIT) && isBitSet(lcdc, LCDC_WIN_DISPLAY_BIT);
}

bool LCDControlAndStat::isSpritesOn() {
    return isBitSet(lcdc, LCDC_SPRITE_DISPLAY_BIT);
}
