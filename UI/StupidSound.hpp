
#ifndef _STUPIDSOUND_HPP_
#define _STUPIDSOUND_HPP_

#include <cstdint>
#include <SDL2/SDL.h>
#include <queue>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "UI/Sound.hpp"

using namespace std;

class StupidSound : public Sound {
private:
    mutex bufferMutex;
    queue<struct AudioBuffer *>channel1BuffersQueue;

public:
    void init();
    void run();

    void pushBuffer(struct AudioBuffer *);
    struct AudioBuffer *popBuffer();
};

#endif