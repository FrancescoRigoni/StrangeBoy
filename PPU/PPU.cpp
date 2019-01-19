
#include <vector>
#include "PPU/PPU.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

#define SCREEN_HEIGHT_PX                            144
#define SCREEN_WIDTH_PX                             160
#define VBLANK_LINES                                 10
#define BACKGROUND_TILE_WIDTH_PX                      8
#define BACKGROUND_TILE_HEIGHT_PX                     8
#define BACKGROUND_TILE_MAP_ROW_SIZE_BYTES           32
#define SCREEN_HEIGHT_INCLUDING_VBLANK              154

#define SPRITE_WIDTH_PX                               8

#define CLOCK_TICKS_LINE_DURING_VBLANK              456
#define CLOCK_TICKS_DURING_OAM_SEARCH                84
#define CLOCK_TICKS_DURING_LINE_DRAW                176
#define CLOCK_TICKS_DURING_HBLANK                   208


PPU::PPU(Memory *memory, 
         LCDRegs *lcdRegs, 
         InterruptFlags *interruptFlags,
         Screen *screen) {

    this->memory = memory;
    this->lcdRegs = lcdRegs;
    this->interruptFlags = interruptFlags;
    this->screen = screen;

    // Initial PPU state
    lcdRegs->stateOAMSearch();
    lcdRegs->write8(LY, 0);
}

int PPU::run() {
    if (lcdRegs->inVBlank()) {
        return CLOCK_TICKS_LINE_DURING_VBLANK;

    } else if (lcdRegs->inOAMSearch()) {
        return CLOCK_TICKS_DURING_OAM_SEARCH;

    } else if (lcdRegs->inDrawLine()) {
        doDrawLine();
        return CLOCK_TICKS_DURING_LINE_DRAW;

    } else if (lcdRegs->inHBlank()) {
        return CLOCK_TICKS_DURING_HBLANK;

    } else {
        TRACE_PPU(endl << "Unknown state for PPU" << endl);
        return INT_MAX;
    }
}

void PPU::nextState() {
    uint8_t stat = lcdRegs->read8(STAT);

    if (lcdRegs->inOAMSearch()) {
        lcdRegs->stateDrawLine();
        // LCDC interrupt is not generated for this state.

    } else if (lcdRegs->inDrawLine()) {
        // Line was drawn, enter HBlank.
        lcdRegs->stateHBlank();

        // Generate LCDC interrupt if the selection says so.
        if (isBitSet(stat, STAT_INTERRUPT_SELECTION_MODE_HBLANK_BIT)) {
            interruptFlags->interruptLCDC();
        }

    } else if (lcdRegs->inHBlank() || 
               lcdRegs->inVBlank()) {
        // Increment line number we are about to draw, reset to line zero if it overflows.
        uint8_t line = lcdRegs->read8(LY);
        line = (line+1)%SCREEN_HEIGHT_INCLUDING_VBLANK;
        lcdRegs->write8(LY, line);

        // Set the ly, lyc coincidence bit if we are about to draw the lyc line.
        uint8_t lyc = lcdRegs->read8(LYC);
        if (line == lyc) setBit(STAT_LYC_LY_COINCIDENCE_BIT, &stat);
        else resetBit(STAT_LYC_LY_COINCIDENCE_BIT, &stat);

        if (line < SCREEN_HEIGHT_PX) {
            // We are still drawing inside the visible screen.
            lcdRegs->stateOAMSearch();

            // Generate LCDC interrupt if the selection says so.
            if (isBitSet(stat, STAT_INTERRUPT_SELECTION_MODE_OAM_SEARCH_BIT)) {
                interruptFlags->interruptLCDC();
            }

        } else if (line == SCREEN_HEIGHT_PX) {
            // We entered VBlank.
            lcdRegs->stateVBlank();
            interruptFlags->interruptVBlank();

            // Generate LCDC interrupt if the selection says so.
            if (isBitSet(stat, STAT_INTERRUPT_SELECTION_MODE_VBLANK_BIT)) {
                interruptFlags->interruptLCDC();
            }
        } // else keep incrementing line until it wraps back to zero
    }
}

void PPU::doDrawLine() {
    if (!lcdRegs->isScreenOn()) {
        TRACE_PPU("Screen is off" << endl);
        uint8_t *pixelsForLine = new uint8_t[SCREEN_WIDTH_PX];
        memset(pixelsForLine, 0, sizeof(uint8_t) * SCREEN_WIDTH_PX);
        screen->pushLine(pixelsForLine);
        return;
    }

    // Read line from LY register
    uint8_t line = lcdRegs->read8(LY);

    if (line < SCREEN_HEIGHT_PX) {
        uint8_t *pixelsForLine = new uint8_t[SCREEN_WIDTH_PX];
        memset(pixelsForLine, sizeof(uint8_t), 0);
        if (lcdRegs->isBackgroundOn()) {
            drawBackgroundPixels(line, pixelsForLine);
        }
        if (lcdRegs->isWindowOn()) {
            drawWindowPixels(line, pixelsForLine);
        }
        if (lcdRegs->isSpritesOn()) {
            drawSpritesPixels(line, pixelsForLine);
        }

        screen->pushLine(pixelsForLine);
    }
}

void PPU::drawBackgroundPixels(int line, uint8_t *pixels) {
    uint8_t scrollY = lcdRegs->read8(SCY);
    uint8_t scrollX = lcdRegs->read8(SCX);
    uint16_t scrolledLine = line;// + scrollY;

    if (scrollX > 0) {
        cout << "ScrollX: " << cout8Hex(scrollX) << endl;
    }

    uint8_t *palette = new uint8_t[4];
    decodePaletteByte(BGP, palette);

    // Go on drawing the tiles pixels for this line
    int rowInTilesMap = scrolledLine/BACKGROUND_TILE_HEIGHT_PX;
    uint16_t tileMapAddress = lcdRegs->addressForBackgroundTilesMap();

    for (int x = 0; x < SCREEN_WIDTH_PX; x++) {
        uint16_t scrolledColumn = scrollX + x;

        int colInTilesMap = scrolledColumn/BACKGROUND_TILE_WIDTH_PX;
        uint16_t tileNumberAddress = tileMapAddress + 
            (rowInTilesMap*BACKGROUND_TILE_MAP_ROW_SIZE_BYTES) + 
            colInTilesMap;
        uint8_t tileNumber = memory->read8(tileNumberAddress, false);

        uint16_t tileAddress = lcdRegs->addressForBackgroundTile(tileNumber);

        int xInTile = scrolledColumn%BACKGROUND_TILE_WIDTH_PX;
        int yInTile = scrolledLine%BACKGROUND_TILE_HEIGHT_PX;    // Calculate positions in tile for current px

        uint16_t yOffsetInTile = yInTile*2;                      // Two bytes per row
        uint8_t xBitInTileBytes = (7-xInTile);                   // msb is first pixel, lsb is last pixel

        uint8_t lsb = memory->read8(tileAddress+yOffsetInTile, false);
        uint8_t msb = memory->read8(tileAddress+yOffsetInTile+1, false);

        uint8_t mask = 1 << xBitInTileBytes;
        uint8_t msbc = ((msb & mask) >> xBitInTileBytes) << 1;
        uint8_t lsbc = (lsb & mask) >> xBitInTileBytes;
        uint8_t color = msbc | lsbc;

        pixels[x] = palette[color];
    }

    delete[] palette;
}

void PPU::drawWindowPixels(int line, uint8_t *pixels) {
    TRACE_PPU("Window enabled" << endl);
}

typedef struct {
    uint8_t yPos;
    uint8_t xPos;
    uint8_t patternNumber;
    uint8_t flags;
} SpriteAttributeEntry;

#define MAX_SPRITES_PER_LINE                  10
#define SPRITE_SCREEN_X(spriteXFromAttrTable) spriteXFromAttrTable - SPRITE_WIDTH_PX
#define SPRITE_SCREEN_Y(spriteYFromAttrTable) spriteYFromAttrTable - 16

#define SPRITE_ATTRIBUTE_Y_FLIP      6
#define SPRITE_ATTRIBUTE_X_FLIP      5
#define SPRITE_ATTRIBUTE_PALETTE_BIT 4

void PPU::drawSpritesPixels(int line, uint8_t *pixels) {
    vector<SpriteAttributeEntry *> spritesEntriesForLine;

    // Find which sprites fall on this line
    for (uint16_t spriteAttribute = SPRITE_ATTRIBUTE_TABLE_START; 
        spriteAttribute < (SPRITE_ATTRIBUTE_TABLE_START + SPRITE_ATTRIBUTE_TABLE_SIZE);
        spriteAttribute += sizeof(SpriteAttributeEntry)) {

        if (spritesEntriesForLine.size() == MAX_SPRITES_PER_LINE) {
            // Stop looking for sprites, we have enough.
            break;
        }

        SpriteAttributeEntry *entry = (SpriteAttributeEntry*) memory->getRawPointer(spriteAttribute);
        int spriteYStartOnScreen = SPRITE_SCREEN_Y(entry->yPos);
        int spriteYEndOnScreen = spriteYStartOnScreen + lcdRegs->spriteHeightPx();
        int lineIsInSpriteYRange = line >= spriteYStartOnScreen && line < spriteYEndOnScreen;

        if (lineIsInSpriteYRange) {
            spritesEntriesForLine.push_back(entry);
        }
    }

    // TODO: sort sprites by x coordinate.

    for (auto spriteAttributes : spritesEntriesForLine) {
        if (SPRITE_SCREEN_X(spriteAttributes->xPos) == 0 || 
            SPRITE_SCREEN_X(spriteAttributes->xPos) >= SCREEN_WIDTH_PX) {
            // TODO: According to the gameboy manual sprites on this line that fall 
            // outside of the screen can still affect the way other sprites on this 
            // line are drawn.
            continue;
        }
               
        uint16_t spriteData = lcdRegs->addressForSprite(spriteAttributes->patternNumber);
        uint8_t lineInSpriteData;

        // Handle vertical flip
        if (isBitClear(spriteAttributes->flags, SPRITE_ATTRIBUTE_Y_FLIP)) {
            lineInSpriteData = lcdRegs->spriteHeightPx() - 
                (spriteAttributes->yPos - lcdRegs->spriteHeightPx() - line);             
        } else {
            lineInSpriteData = spriteAttributes->yPos - lcdRegs->spriteHeightPx() - line;
        }

        uint16_t addressOfLineInSpriteData = spriteData + (lineInSpriteData*2);
        uint8_t lsb = memory->read8(addressOfLineInSpriteData, false);
        uint8_t msb = memory->read8(addressOfLineInSpriteData+1, false);

        int pixelPositionInLine = SPRITE_SCREEN_X(spriteAttributes->xPos);

        uint8_t *palette = new uint8_t[4];
        if (isBitSet(spriteAttributes->flags, SPRITE_ATTRIBUTE_PALETTE_BIT)) 
            decodePaletteByte(OBJ_PAL_1, palette);
        else 
            decodePaletteByte(OBJ_PAL_0, palette);

        // Handle horizontal flip
        bool xFlipped = isBitSet(spriteAttributes->flags, SPRITE_ATTRIBUTE_X_FLIP);
        for (int i = (xFlipped ? 0 : 7); 
                 i != (xFlipped ? 8 : -1); 
                 i += (xFlipped ? 1 : -1)) {

            if (pixelPositionInLine > 0 && 
                pixelPositionInLine < SCREEN_WIDTH_PX) {

                uint8_t mask = 1 << i;
                uint8_t msbc = ((msb & mask) >> i) << 1;
                uint8_t lsbc = (lsb & mask) >> i;
                uint8_t color = msbc | lsbc;

                // TODO: Handle transparent color and priority flag
                if (color > 0) pixels[pixelPositionInLine] = palette[color];
            }
            if (++pixelPositionInLine > SCREEN_WIDTH_PX) {
                // Sprite goes off the screen, stop drawing it
                break;
            }
        }
        
        delete[] palette;
    }
}

void PPU::decodePaletteByte(uint16_t address, uint8_t *palette) {
    uint8_t paletteByte = memory->read8(address);
    palette[0] = paletteByte & 0b00000011;
    palette[1] = (paletteByte & 0b00001100) >> 2;
    palette[2] = (paletteByte & 0b00110000) >> 4;
    palette[3] = (paletteByte & 0b11000000) >> 6;
}
