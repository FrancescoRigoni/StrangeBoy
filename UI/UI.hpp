
#ifndef _UI_HPP_
#define _UI_HPP_

#include <SDL2/SDL.h>
#include "UI/StupidScreen.hpp"
#include "UI/StupidSound.hpp"
#include "Devices/Joypad.hpp"

class UI {
private:
    StupidScreen screen;
    StupidSound sound;
    Joypad joypad;

    bool pollEvent();

public:
    StupidScreen *getScreen();
    StupidSound *getSound();
    Joypad *getJoypad();

    void run();
};

#endif