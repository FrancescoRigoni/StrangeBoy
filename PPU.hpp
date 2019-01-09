
#include "Io.hpp"
#include "Memory.hpp"
#include "ByteUtil.hpp"
#include "Screen.hpp"
#include "LCDControlAndStat.hpp"
#include "InterruptFlags.hpp"

#include <cstdint>

class PPU {
private:
    Memory * memory;
    Screen * screen;
    LCDControlAndStat *lcdControlAndStat;
    InterruptFlags *interruptFlags;

    void drawBackgroundPixels(int, uint8_t *);
    void drawWindowPixels(int, uint8_t *);
    void drawSpritesPixels(int, uint8_t *);
    void doDrawLine();

    uint8_t *decodeBackgroundPalette();

public:
    PPU(Memory *, LCDControlAndStat *, InterruptFlags *, Screen *);
    ~PPU();

    int run();
    void nextState();
};