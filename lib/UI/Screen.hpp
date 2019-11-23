
#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <cstdint>

class Screen {
public:
    virtual void resetToFirstLine() = 0;
    virtual void pushLine(uint8_t *) = 0;
    virtual void init() = 0;
    virtual void run() = 0;
    virtual void quit() = 0;
};

#endif