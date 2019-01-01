
#include "Io.hpp"
#include "Memory.hpp"
#include "ByteUtil.hpp"

#include <cstdint>

// LCDC
#define LCDC_CONTROL_OP                     1 << 7
#define LCDC_WIN_TILE_MAP_DISPLAY_SELECT    1 << 6
#define LCDC_WIN_DISPLAY                    1 << 5
#define LCDC_BG_AND_WIN_TILE_DATA_SELECT    1 << 4
#define LCDC_BG_TILE_MAP_DISPLAY_SELECT     1 << 3
#define LCDC_SPRITE_SIZE                    1 << 2
#define LCDC_BG_AND_WIN_DISPLAY             1

class Display {
private:
    Memory * memory;

public:
    Display(Memory * memory) {
        this->memory = memory;
    }

    // Note: opposite of what the gameboy manual says
    uint16_t bgTilesDataAddress() {
        uint8_t lcdc = memory->read8(LCDC);
        return !isBitSet(lcdc, LCDC_BG_AND_WIN_TILE_DATA_SELECT) ? 0x8000 : 0x8800;
    }

    uint16_t bgTilesMapAddress() {
        uint8_t lcdc = memory->read8(LCDC);
        return isBitSet(lcdc, LCDC_BG_TILE_MAP_DISPLAY_SELECT) ? 0x9C00 : 0x9800;
    }

    void dumpBgTilesMap();
    void dumpBgTilesData();
    void renderTile(uint16_t address);
};