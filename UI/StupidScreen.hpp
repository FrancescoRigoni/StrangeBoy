
#ifndef __STUPIDSCREEN_H__
#define __STUPIDSCREEN_H__

#include <cstdint>
#include <SDL2/SDL.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <UI/Screen.hpp>

using namespace std;

class StupidScreen : public Screen {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int currentScanLine = 0;

    mutex linesMutex;
    condition_variable zeroLinesCondition;
    condition_variable tooManyLinesCondition;
    queue<uint8_t *> linesQueue;

    void drawNextLine();

public:
    void resetToFirstLine();
    void pushLine(uint8_t *);
    uint8_t *popLine();
    void run();

    void init();
    void quit();
};

#endif