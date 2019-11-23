
#include "UI/UI.hpp"

StupidScreen *UI::getScreen() {
    return &screen;
}

Joypad *UI::getJoypad() {
    return &joypad;
}

StupidSound *UI::getSound() {
    return &sound;
}

void UI::run() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    screen.init();
    sound.init();

    while (!pollEvent()) {
        screen.run();
        sound.run();
    }

    screen.quit();
}

bool UI::pollEvent() {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return true;
        } else if (event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_a) {
                joypad.buttonA = true;
            } else if(event.key.keysym.sym == SDLK_z) {
                joypad.buttonB = true;
            } else if(event.key.keysym.sym == SDLK_s) {
                joypad.buttonStart = true;
            } else if(event.key.keysym.sym == SDLK_x) {   
                joypad.buttonSelect = true;
            } else if(event.key.keysym.sym == SDLK_UP) {
                joypad.buttonUp = true;
            } else if(event.key.keysym.sym == SDLK_DOWN) {
                joypad.buttonDown = true;
            } else if(event.key.keysym.sym == SDLK_LEFT) {
                joypad.buttonLeft = true;
            } else if(event.key.keysym.sym == SDLK_RIGHT) {
                joypad.buttonRight = true;
            }
        } else if (event.type == SDL_KEYUP) {
            if(event.key.keysym.sym == SDLK_a) {
                joypad.buttonA = false;
            } else if(event.key.keysym.sym == SDLK_z) {
                joypad.buttonB = false;
            } else if(event.key.keysym.sym == SDLK_s) {
                joypad.buttonStart = false;
            } else if(event.key.keysym.sym == SDLK_x) { 
                joypad.buttonSelect = false;  
            } else if(event.key.keysym.sym == SDLK_UP) {
                joypad.buttonUp = false;
            } else if(event.key.keysym.sym == SDLK_DOWN) {
                joypad.buttonDown = false;
            } else if(event.key.keysym.sym == SDLK_LEFT) {
                joypad.buttonLeft = false;
            } else if(event.key.keysym.sym == SDLK_RIGHT) {
                joypad.buttonRight = false;
            }
        }
    }
    return false;
}