
#include "PPU.hpp"
#include "LogUtil.hpp"
#include "ByteUtil.hpp"

#define SCREEN_HEIGHT_PX                            144
#define SCREEN_WIDTH_PX                             160
#define VBLANK_LINES                                 10
#define BACKGROUND_TILE_WIDTH_PX                      8
#define BACKGROUND_TILE_HEIGHT_PX                     8
#define BACKGROUND_TILE_MAP_ROW_SIZE_BYTES           32
#define SCREEN_HEIGHT_INCLUDING_VBLANK              154

#define CLOCK_TICKS_LINE_DURING_VBLANK      456
#define CLOCK_TICKS_DURING_OAM_SEARCH        84
#define CLOCK_TICKS_DURING_LINE_DRAW        176
#define CLOCK_TICKS_DURING_HBLANK           208


PPU::PPU(Memory * memory, LCDControlAndStat * lcdControlAndStat, InterruptFlags * interruptFlags) {
    this->memory = memory;
    this->lcdControlAndStat = lcdControlAndStat;
    this->interruptFlags = interruptFlags;
    screen = new Screen();

    lcdControlAndStat->stateVBlank();
    lcdControlAndStat->write8(LY, SCREEN_HEIGHT_INCLUDING_VBLANK-1);
}

PPU::~PPU() {
    delete screen;
}

int PPU::run() {
    if (lcdControlAndStat->inVBlank()) {
        return CLOCK_TICKS_LINE_DURING_VBLANK;

    } else if (lcdControlAndStat->inOAMSearch()) {
        return CLOCK_TICKS_DURING_OAM_SEARCH;

    } else if (lcdControlAndStat->inDrawLine()) {
        doDrawLine();
        return CLOCK_TICKS_DURING_LINE_DRAW;

    } else if (lcdControlAndStat->inHBlank()) {
        return CLOCK_TICKS_DURING_HBLANK;

    } else {
        TRACE_PPU(endl << "Unknown state for PPU" << endl);
        return INT_MAX;
    }
}

void PPU::nextState() {
    uint8_t stat = lcdControlAndStat->read8(STAT);

    if (lcdControlAndStat->inOAMSearch()) {
        lcdControlAndStat->stateDrawLine();
        // LCDC interrupt is not generated for this state.

    } else if (lcdControlAndStat->inDrawLine()) {
        // Line was drawn, enter HBlank.
        lcdControlAndStat->stateHBlank();

        // Generate LCDC interrupt if the selection says so.
        if (isBitSet(stat, STAT_INTERRUPT_SELECTION_MODE_HBLANK_BIT)) {
            interruptFlags->interruptLCDC();
        }

    } else if (lcdControlAndStat->inHBlank() || 
               lcdControlAndStat->inVBlank()) {
        // Increment line number we are about to draw, reset to line zero if it overflows.
        uint8_t line = lcdControlAndStat->read8(LY);
        line = (line+1)%SCREEN_HEIGHT_INCLUDING_VBLANK;
        lcdControlAndStat->write8(LY, line);

        // Set the ly, lyc coincidence bit if we are about to draw the lyc line.
        uint8_t lyc = lcdControlAndStat->read8(LYC);
        if (line == lyc) setBit(STAT_LYC_LY_COINCIDENCE_BIT, &stat);
        else resetBit(STAT_LYC_LY_COINCIDENCE_BIT, &stat);

        if (line < SCREEN_HEIGHT_PX) {
            // We are still drawing inside the visible screen.
            lcdControlAndStat->stateOAMSearch();

            // Generate LCDC interrupt if the selection says so.
            if (isBitSet(stat, STAT_INTERRUPT_SELECTION_MODE_OAM_SEARCH_BIT)) {
                interruptFlags->interruptLCDC();
            }

        } else if (line == SCREEN_HEIGHT_PX) {
            // We entered VBlank.
            lcdControlAndStat->stateVBlank();
            interruptFlags->interruptVBlank();

            // Generate LCDC interrupt if the selection says so.
            if (isBitSet(stat, STAT_INTERRUPT_SELECTION_MODE_VBLANK_BIT)) {
                interruptFlags->interruptLCDC();
            }
        }
    }
}

void PPU::doDrawLine() {
    if (!lcdControlAndStat->isScreenOn()) {
        TRACE_PPU("Screen is off" << endl);
        screen->nextLine();
        return;
    }

    // Read line from LY register
    uint8_t line = lcdControlAndStat->read8(LY);

    if (line < SCREEN_HEIGHT_PX) {
        uint8_t *pixelsForLine = new uint8_t[SCREEN_WIDTH_PX];
        memset(pixelsForLine, sizeof(uint8_t), 0);
        if (lcdControlAndStat->isBackgroundOn()) {
            drawBackgroundPixels(line, pixelsForLine);
        }
        if (lcdControlAndStat->isWindowOn()) {
            drawWindowPixels(line, pixelsForLine);
        }
        if (lcdControlAndStat->isSpritesOn()) {
            drawSpritesPixels(line, pixelsForLine);
        }

        screen->sendLine(pixelsForLine);
        delete[] pixelsForLine;
    }
}

void PPU::drawBackgroundPixels(int line, uint8_t *pixels) {
    uint8_t scrollY = lcdControlAndStat->read8(SCY);
    uint16_t scrolledLine = line + scrollY;

    uint8_t *backgroundPalette = decodeBackgroundPalette();

    // Go on drawing the tiles pixels for this line
    int rowInTilesMap = scrolledLine/BACKGROUND_TILE_HEIGHT_PX;
    uint16_t tileMapAddress = lcdControlAndStat->addressForBackgroundTilesMap();

    for (int x = 0; x < SCREEN_WIDTH_PX; x++) {

        int colInTilesMap = x/BACKGROUND_TILE_WIDTH_PX;
        uint16_t tileNumberAddress = tileMapAddress + 
            (rowInTilesMap*BACKGROUND_TILE_MAP_ROW_SIZE_BYTES) + 
            colInTilesMap;
        uint8_t tileNumber = memory->read8(tileNumberAddress, false);

        uint16_t tileAddress = lcdControlAndStat->addressForBackgroundTile(tileNumber);

        int xInTile = x%BACKGROUND_TILE_WIDTH_PX;
        int yInTile = scrolledLine%BACKGROUND_TILE_HEIGHT_PX;    // Calculate positions in tile for current px

        uint16_t yOffsetInTile = yInTile*2;                      // Two bytes per row
        uint8_t xBitInTileBytes = (7-xInTile);                   // msb is first pixel, lsb is last pixel

        uint8_t lsb = memory->read8(tileAddress+yOffsetInTile, false);
        uint8_t msb = memory->read8(tileAddress+yOffsetInTile+1, false);

        uint8_t mask = 1 << xBitInTileBytes;
        uint8_t msbc = ((msb & mask) >> xBitInTileBytes) << 1;
        uint8_t lsbc = (lsb & mask) >> xBitInTileBytes;
        uint8_t color = msbc | lsbc;

        pixels[x] = backgroundPalette[color];
    }
}

void PPU::drawWindowPixels(int line, uint8_t *pixels) {
    TRACE_PPU("Window enabled" << endl);
}

void PPU::drawSpritesPixels(int line, uint8_t *pixels) {
    //TRACE_PPU("Sprites enabled" << endl);
}

uint8_t *PPU::decodeBackgroundPalette() {
    uint8_t *backgroundPalette = new uint8_t[4];
    uint8_t backgroundPaletteByte = memory->read8(BGP);
    backgroundPalette[0] = backgroundPaletteByte & 0b00000011;
    backgroundPalette[1] = (backgroundPaletteByte & 0b00001100) >> 2;
    backgroundPalette[2] = (backgroundPaletteByte & 0b00110000) >> 4;
    backgroundPalette[3] = (backgroundPaletteByte & 0b11000000) >> 6;
    return backgroundPalette;
}
