
#include "Display.hpp"
#include "LogUtil.hpp"

void Display::dumpBgTilesMap() {
    uint16_t baseAddress = bgTilesMapAddress();
    for (int y=0; y<32; y++) {
        for (int x=0; x<32; x++) {
            uint8_t tileNumber = memory->read8(baseAddress+(y*32)+x);
            cout << cout8Hex(tileNumber) << " ";
        }
        cout << endl;
    }
    cout << endl;
}

void Display::dumpBgTilesData() {
    uint16_t baseAddress = bgTilesDataAddress();
    for (int a=0; a<0x1000; a+=16) {
        int sum = 0;
        for (int i = 0; i < 16; i++) {
            sum += memory->read8(baseAddress+a+i);
        }
        if (sum > 0) renderTile(baseAddress+a);
    }
    cout << "---------------------------------------" << endl;
}

/*
90b$08
TN: $990c$09
TN: $990d$0a
TN: $990e$0b
TN: $990f$0c
TN: $9910$19
TN: $9911$00
TN: $9912$00
TN: $9913$00
TN: $9914$00
TN: $9915$00
TN: $9916$00
TN: $9917$00
TN: $9918$00
TN: $9919$00
TN: $991a$00
TN: $991b$00
TN: $991c$00
TN: $991d$00
TN: $991e$00
TN: $991f$00
*/

void Display::drawBackground() {
    uint16_t tileMapAddress = bgTilesMapAddress();
    for (int y = 0; y < 144; y++) {
        int rowInTilesMap = y/8; // Each tile is 8 px tall
        bool hasTile = false;
        for (int x = 0; x < 160; x++) {
            int colInTilesMap = x/8; // Each tile is 8 px wide
            uint16_t tileNumberAddress = tileMapAddress + (rowInTilesMap*32)+colInTilesMap;
            uint8_t tileNumber = memory->read8(tileNumberAddress);

            if (tileNumber != 0) {
                //cout << "TN: " << cout16Hex(tileNumberAddress) << cout8Hex(tileNumber) << endl;
                hasTile = true;
                uint16_t tileAddress = addressForTile(tileNumber);       // Retrieve tile

                int xInTile = x%8, yInTile = y%8;                        // Calculate positions in tile for current px
                uint16_t yOffsetInTile = yInTile*2;                      // Two bytes per row
                uint8_t xBitInTileBytes = (7-xInTile);                   // msb is first pixel, lsb is last pixel

                uint8_t lsb = memory->read8(tileAddress+yOffsetInTile);
                uint8_t msb = memory->read8(tileAddress+yOffsetInTile+1);

                uint8_t mask = 1 << xBitInTileBytes;
                uint8_t msbc = (msb & mask) >> (xBitInTileBytes-1);
                uint8_t lsbc = (lsb & mask) >> xBitInTileBytes;
                uint8_t color = msbc | lsbc;

                if (color == 1) cout << "O";
                else if (color == 2) cout << "x";
                else if (color == 3) cout << "X";
                else if (color == 0) cout << " ";
            }
        }

        if (hasTile) cout << endl;
    }

    cout << endl;
}

void Display::renderTile(uint16_t address) {
    for (int p = 0; p < 8; p++) {
        uint8_t lsb = memory->read8(address);
        uint8_t msb = memory->read8(address+1);
        for (int i = 7; i >= 0; i--) {
            uint8_t mask = 1 << i;
            uint8_t msbc = (msb & mask) >> (i-1);
            uint8_t lsbc = (lsb & mask) >> i;
            uint8_t color = msbc | lsbc;
            if (color == 0) cout << " ";
            else if (color == 1) cout << ".";
            else if (color == 2) cout << "x";
            else if (color == 3) cout << "X";
            else cout << "_";
        }
        address += 2;
        cout << endl;
    }

    cout << "--------" << endl;

}

// Note: opposite of what the gameboy manual says
uint16_t Display::bgTilesDataAddress() {
    uint8_t lcdc = memory->read8(LCDC);
    return !isBitSet(lcdc, LCDC_BG_AND_WIN_TILE_DATA_SELECT) ? 0x8000 : 0x8800;
}

// Note: opposite of what the gameboy manual says
uint16_t Display::addressForTile(int8_t tileNumber) {
    uint8_t lcdc = memory->read8(LCDC);
    if (!isBitSet(lcdc, LCDC_BG_AND_WIN_TILE_DATA_SELECT)) {
        // Unsigned offset
        uint16_t uTileNumber = (uint16_t)tileNumber;
        uint16_t uTileOffset = uTileNumber * 16;
        return bgTilesDataAddress() + uTileOffset;
    } else {
        // Signed offset
        int16_t sTileNumber = tileNumber;
        int16_t sTileOffset = sTileNumber * 16;
        return bgTilesDataAddress() + sTileOffset;
    }
}

uint16_t Display::bgTilesMapAddress() {
    uint8_t lcdc = memory->read8(LCDC);
    return isBitSet(lcdc, LCDC_BG_TILE_MAP_DISPLAY_SELECT) ? 0x9C00 : 0x9800;
}
