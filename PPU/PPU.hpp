
#include "Devices/Io.hpp"
#include "Cpu/Memory.hpp"
#include "Util/ByteUtil.hpp"
#include "UI/Screen.hpp"
#include "Devices/LCDRegs.hpp"
#include "Devices/InterruptFlags.hpp"

#include <cstdint>

class PPU {
private:
    Memory *memory;
    Screen *screen;
    LCDRegs *lcdRegs;
    InterruptFlags *interruptFlags;

    void drawBackgroundPixels(int, uint8_t *);
    void drawWindowPixels(int, uint8_t *);
    void drawSpritesPixels(int, uint8_t *);
    void doDrawLine();

    void decodePaletteByte(uint16_t, uint8_t *);
    uint8_t decodePixelFromTile(uint16_t, int, int, int, int, uint8_t *);

    int currentWindowLine;

public:
    PPU(Memory *, LCDRegs *, InterruptFlags *, Screen *);

    int run();
    void nextState();
};