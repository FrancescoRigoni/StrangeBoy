
#ifndef _UI_HPP_
#define _UI_HPP_

#include "Screen/Screen.hpp"
#include "Devices/Joypad.hpp"
#include <SDL2/SDL.h>

class UI {
private:
    Screen screen;
    Joypad joypad;

    bool pollEvent();

public:
    Screen *getScreen();
    Joypad *getJoypad();

    void run();
};

#endif