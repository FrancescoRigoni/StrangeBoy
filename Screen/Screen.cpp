
#include <iostream>
#include "Screen/Screen.hpp"
#include "Util/LogUtil.hpp"

#define SCREEN_HEIGHT_PX                            144
#define SCREEN_WIDTH_PX                             160

#define TOO_MANY_LINES_QUEUED          SCREEN_HEIGHT_PX
#define RENDER_SCALE                               2.5f

#define TILE_WIDTH_PX                                 8
#define TILE_HEIGHT_PX                                8

#define DRAW_TILES_GRID false

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
        (SCREEN_WIDTH_PX + (DRAW_TILES_GRID ? 1 : 0)) * RENDER_SCALE,
        (SCREEN_HEIGHT_PX + (DRAW_TILES_GRID ? 1 : 0)) * RENDER_SCALE,
        0
    );

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetScale(renderer, RENDER_SCALE, RENDER_SCALE);
    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    bool isquit = false;
    while (!isquit) {
        if (pollJoypadEvent()) return;
        drawNextLine();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Screen::drawNextLine() {
    uint8_t *pixels = popLine();
    if (pixels == 0) return;

    SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);

    for (int i = 0; i < SCREEN_WIDTH_PX; i++) {
        if (pixels[i] == 0) SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        else if (pixels[i] == 1) SDL_SetRenderDrawColor(renderer, 160, 160, 160, 255);
        else if (pixels[i] == 2) SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        else if (pixels[i] == 3) SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);

        SDL_RenderDrawPoint(renderer, i, currentScanLine);
    }

    currentScanLine++;
    currentScanLine %= SCREEN_HEIGHT_PX;

    if (DRAW_TILES_GRID) {
        if (currentScanLine == 0) {
            // Entire screen drawn.
            SDL_SetRenderDrawColor(renderer, 100, 0, 150, 255);
            for (int x = 0; x <= SCREEN_WIDTH_PX; x+=TILE_WIDTH_PX) {
                SDL_Point points[2] = {{x, 0}, {x, SCREEN_HEIGHT_PX}};
                SDL_RenderDrawLines(renderer, points, 2);
            }

            for (int y = 0; y <= SCREEN_HEIGHT_PX; y+=TILE_HEIGHT_PX) {
                SDL_Point points[2] = {{0, y}, {SCREEN_WIDTH_PX, y}};
                SDL_RenderDrawLines(renderer, points, 2);
            }
        }
    }

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

bool Screen::pollJoypadEvent() {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
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
    return false;
}
