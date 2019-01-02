
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

    uint16_t bgTilesDataAddress();
    uint16_t addressForTile(int8_t tileNumber);
    uint16_t bgTilesMapAddress();

    void drawLine();
    void drawScreen();
    void renderTile(uint16_t address);
};