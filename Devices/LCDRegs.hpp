
#ifndef __LCDC_H__
#define __LCDC_H__

#include <cstdint>
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"

#define LCDC                0xFF40 
#define STAT                0xFF41
#define SCY                 0xFF42 
#define SCX                 0xFF43
#define LY                  0xFF44
#define LYC                 0xFF45
#define WIN_Y               0xFF4A
#define WIN_X               0xFF4B

#define LCDC_CONTROL_OP_BIT                                 7
#define LCDC_WIN_TILE_MAP_DISPLAY_SELECT_BIT                6
#define LCDC_WIN_DISPLAY_BIT                                5
#define LCDC_BG_AND_WIN_TILE_DATA_SELECT_BIT                4
#define LCDC_BG_TILE_MAP_DISPLAY_SELECT_BIT                 3
#define LCDC_SPRITE_SIZE_BIT                                2
#define LCDC_SPRITE_DISPLAY_BIT                             1
#define LCDC_BG_AND_WIN_DISPLAY_BIT                         0

#define STAT_INTERRUPT_SELECTION_LYC_LY_COINCIDENCE_BIT     6
#define STAT_INTERRUPT_SELECTION_MODE_OAM_SEARCH_BIT        5
#define STAT_INTERRUPT_SELECTION_MODE_VBLANK_BIT            4
#define STAT_INTERRUPT_SELECTION_MODE_HBLANK_BIT            3
#define STAT_LYC_LY_COINCIDENCE_BIT                         2

#define SPRITE_ATTRIBUTE_TABLE_START        0xFE00
#define SPRITE_ATTRIBUTE_TABLE_SIZE         0x00A0
#define SPRITE_PATTERN_TABLE_START          0x8000

class LCDRegs : public IoDevice {
private:
    uint8_t lcdc;
    uint8_t stat;
    int8_t scy;
    int8_t scx;
    uint8_t ly = 0;
    uint8_t lyc;
    uint8_t windowY;
    uint8_t windowX;

public:
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    uint16_t addressForBackgroundTile(uint8_t);
    uint16_t addressForBackgroundTilesMap();

    uint16_t addressForWindowTile(uint8_t);
    uint16_t addressForWindowTilesMap();

    uint16_t addressForSprite(uint16_t);
    uint16_t addressForSpriteAttributeTable();

    void stateOAMSearch();
    void stateDrawLine();
    void stateHBlank();
    void stateVBlank();

    bool inOAMSearch();
    bool inDrawLine();
    bool inHBlank();
    bool inVBlank();

    bool isScreenOn();
    bool isBackgroundOn();
    bool isWindowOn();
    bool isSpritesOn();

    int spriteHeightPx();
};

#endif