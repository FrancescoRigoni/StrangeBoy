
#include <iostream>
#include "UI/StupidScreen.hpp"
#include "Util/LogUtil.hpp"

#define SCREEN_HEIGHT_PX                            144
#define SCREEN_WIDTH_PX                             160

#define TOO_MANY_LINES_QUEUED          SCREEN_HEIGHT_PX
#define RENDER_SCALE                               4.0f

#define TILE_WIDTH_PX                                 8
#define TILE_HEIGHT_PX                                8

#define DRAW_TILES_GRID false

void StupidScreen::init() {
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
}

void StupidScreen::quit() {
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void StupidScreen::run() {
    drawNextLine();
}

void StupidScreen::resetToFirstLine() {
    unique_lock<mutex> lock(linesMutex);
    currentScanLine = 0;
    while (!linesQueue.empty())
    {
        uint8_t *line = linesQueue.front();
        delete[] line;
        linesQueue.pop();
    }
}

void StupidScreen::drawNextLine() {
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

void StupidScreen::pushLine(uint8_t *pixels) {
    unique_lock<mutex> lock(linesMutex);

    if (linesQueue.size() >= TOO_MANY_LINES_QUEUED) {
        TRACE_SCREEN("Warning, too many lines queued for screen, total " << (int16_t)linesQueue.size() << endl);
    }

    linesQueue.push(pixels);
    zeroLinesCondition.notify_one();
}

uint8_t *StupidScreen::popLine() {
    unique_lock<mutex> lock(linesMutex);

    if (linesQueue.size() == 0) {
        return 0;
    }

    uint8_t *nextLine = linesQueue.front();
    linesQueue.pop();
    return nextLine;
}
