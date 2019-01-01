
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