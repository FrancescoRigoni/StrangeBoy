
#include <cstdint>
#include <SDL2/SDL.h>

class Screen {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int currentScanLine = 0;
public:
    Screen();
    ~Screen();

    void sendLine(uint8_t *pixels);
    void nextLine();
};