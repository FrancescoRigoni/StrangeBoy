
#include <iostream>
#include "Screen/Screen.hpp"
#include "Util/LogUtil.hpp"

#define SCREEN_HEIGHT_PX                            144
#define SCREEN_WIDTH_PX                             160

#define TOO_MANY_LINES_QUEUED          SCREEN_HEIGHT_PX

Screen::Screen(Joypad *joypad) {
    this->joypad = joypad;
}

void Screen::run() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow
    (
        "StrangeBoy", 
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
            } else if (event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym == SDLK_a) {
                    joypad->buttonA = true;
                } else if(event.key.keysym.sym == SDLK_z) {
                    joypad->buttonB = true;
                } else if(event.key.keysym.sym == SDLK_s) {
                    joypad->buttonStart = true;
                } else if(event.key.keysym.sym == SDLK_x) {   
                    joypad->buttonSelect = true;
                } else if(event.key.keysym.sym == SDLK_UP) {
                    joypad->buttonUp = true;
                } else if(event.key.keysym.sym == SDLK_DOWN) {
                    joypad->buttonDown = true;
                } else if(event.key.keysym.sym == SDLK_LEFT) {
                    joypad->buttonLeft = true;
                } else if(event.key.keysym.sym == SDLK_RIGHT) {
                    joypad->buttonRight = true;
                }
            } else if (event.type == SDL_KEYUP) {
                if(event.key.keysym.sym == SDLK_a) {
                    joypad->buttonA = false;
                } else if(event.key.keysym.sym == SDLK_z) {
                    joypad->buttonB = false;
                } else if(event.key.keysym.sym == SDLK_s) {
                    joypad->buttonStart = false;
                } else if(event.key.keysym.sym == SDLK_x) { 
                    joypad->buttonSelect = false;  
                } else if(event.key.keysym.sym == SDLK_UP) {
                    joypad->buttonUp = false;
                } else if(event.key.keysym.sym == SDLK_DOWN) {
                    joypad->buttonDown = false;
                } else if(event.key.keysym.sym == SDLK_LEFT) {
                    joypad->buttonLeft = false;
                } else if(event.key.keysym.sym == SDLK_RIGHT) {
                    joypad->buttonRight = false;
                }
            }
        }

        drawNextLine();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Screen::drawNextLine() {
    uint8_t *pixels = popLine();
    if (pixels == 0) return;

    SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);

    SDL_Rect r;
    r.y = currentScanLine * 2;
    r.w = 2;
    r.h = 2;
    for (int i = 0; i < 160; i++) {
        r.x = i*2;
        if (pixels[i] == 0) SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        else if (pixels[i] == 1) SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
        else if (pixels[i] == 2) SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        else if (pixels[i] == 3) SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);

        SDL_RenderFillRect(renderer, &r);

    }

    currentScanLine++;
    currentScanLine %= SCREEN_HEIGHT_PX;

    if (currentScanLine == 0) {
        SDL_RenderPresent(renderer);
    }

    delete[] pixels;
}

void Screen::pushLine(uint8_t *pixels) {
    unique_lock<mutex> lock(linesMutex);

    if (linesQueue.size() >= TOO_MANY_LINES_QUEUED) {
        TRACE_SCREEN("Warning, too many lines queued for screen, total " << (int16_t)linesQueue.size() << endl);
    }

    linesQueue.push(pixels);
    zeroLinesCondition.notify_one();
}

uint8_t *Screen::popLine() {
    unique_lock<mutex> lock(linesMutex);

    if (linesQueue.size() == 0) {
        return 0;
    }

    uint8_t *nextLine = linesQueue.front();
    linesQueue.pop();
    return nextLine;
}
