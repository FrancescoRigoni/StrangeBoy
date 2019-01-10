
#include "Io.hpp"
#include "Memory.hpp"
#include "ByteUtil.hpp"
#include "Screen.hpp"
#include "LCDRegs.hpp"
#include "InterruptFlags.hpp"

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

    uint8_t *decodeBackgroundPalette();
    uint8_t *decodeSpritePalette0();
    uint8_t *decodeSpritePalette1();

public:
    PPU(Memory *, LCDRegs *, InterruptFlags *, Screen *);

    int run();
    void nextState();
};