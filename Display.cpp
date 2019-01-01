
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
    for (int a=0; a<0x1000; a+=0x10) {
        uint8_t tileData = memory->read8(baseAddress+a);
        if (tileData != 0) renderTile(baseAddress+a);
        // if (tileData != 0) {
        //      printedSomething = true;
        //      cout << cout8Hex(tileData) << " ";
        // }
    }
    cout << "---------------------------------------" << endl;
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