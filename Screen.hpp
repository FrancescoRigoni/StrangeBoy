
#include <cstdint>
#include <SDL2/SDL.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

using namespace std;

class Screen {
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
    void pushLine(uint8_t *);
    uint8_t *popLine();
    void run();
};