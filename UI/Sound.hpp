
#ifndef _SOUND_HPP_
#define _SOUND_HPP_

#include <cstdint>
#include <SDL2/SDL.h>
#include <queue>
#include <mutex>
#include <queue>
#include <condition_variable>

using namespace std;

struct AudioBuffer {
    uint16_t size;
    uint16_t *buffer;
};

class Sound {
private:
    mutex bufferMutex;
    queue<struct AudioBuffer *>channel1BuffersQueue;

    //uint16_t freq;

public:
    void init();
    void run();

    void pushBuffer(struct AudioBuffer *);
    struct AudioBuffer *popBuffer();

    // void pushFreq(int freq);
    // int popFreq();
};

#endif