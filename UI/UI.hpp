
#ifndef _UI_HPP_
#define _UI_HPP_

#include <SDL2/SDL.h>
#include "UI/Screen.hpp"
#include "UI/Sound.hpp"
#include "Devices/Joypad.hpp"

class UI {
private:
    Screen screen;
    Sound sound;
    Joypad joypad;

    bool pollEvent();

public:
    Screen *getScreen();
    Sound *getSound();
    Joypad *getJoypad();

    void run();
};

#endif