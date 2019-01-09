
#include "Screen.hpp"
#include <iostream>

#define SCREEN_HEIGHT_PX                            144
#define SCREEN_WIDTH_PX                             160

void Screen::run() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow
    (
        "Gameboy Emulator", 
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        320,
        288,
        0
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    bool isquit = false;
    SDL_Event event;
    while (!isquit) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isquit = true;
            }
        }

        drawNextLine();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Screen::drawNextLine() {
    uint8_t *pixels = popLine();

    SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);

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

    // if (currentScanLine%8 == 0) {
    //     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    //     SDL_RenderDrawLine(renderer, 0, currentScanLine*2, 320, currentScanLine*2);
    // }

    currentScanLine++;
    currentScanLine %= 144;

    if (currentScanLine == 0) {
        SDL_RenderPresent(renderer);
    }

    delete[] pixels;
}

void Screen::pushLine(uint8_t *pixels) {
    unique_lock<mutex> lock(linesMutex);

    if (linesQueue.size() > 144) {
        tooManyLinesCondition.wait(lock);
    }

    //cout << "Pushing line, total " << (int16_t)linesQueue.size() << endl;

    linesQueue.push(pixels);
    zeroLinesCondition.notify_one();
}

uint8_t *Screen::popLine() {
    unique_lock<mutex> lock(linesMutex);

    if (linesQueue.size() == 0) {
        zeroLinesCondition.wait(lock);
    }

    uint8_t *nextLine = linesQueue.front();
    linesQueue.pop();

    if (linesQueue.size() == (144/2)) {
        tooManyLinesCondition.notify_one();
    }

    //cout << "Popped line, total " << (int16_t)linesQueue.size() << endl;
    return nextLine;
}
