
#include "Screen.hpp"

void Screen::sendLine(uint8_t *pixels) {
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);

    SDL_Rect r;
    r.y = currentScanLine * 2;
    r.w = 2;
    r.h = 2;
    for (int i = 0; i < 160; i++) {
        r.x = i*2;
        if (pixels[i] == 0) SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
        else if (pixels[i] == 1) SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        else if (pixels[i] == 2) SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        else if (pixels[i] == 3) SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);

        SDL_RenderFillRect(renderer, &r);

    }

    currentScanLine++;
    currentScanLine %= 144;

    if (currentScanLine == 0) {
        SDL_RenderPresent(renderer);
    }
}

Screen::Screen() {
    window = SDL_CreateWindow
    (
        "Jeu de la vie", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        320,
        288,
        SDL_WINDOW_SHOWN
    );

    // Setup renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Set render color to red ( background will be rendered in this color )
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);

    // Clear winow
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}

Screen::~Screen() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}