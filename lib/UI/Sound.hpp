
#ifndef _SOUND_HPP_
#define _SOUND_HPP_

#include <cstdint>

struct AudioBuffer {
    uint16_t size;
    uint16_t *buffer;
};

class Sound {
public:
    virtual void init() = 0;
    virtual void run() = 0;
    virtual void pushBuffer(struct AudioBuffer *) = 0;
};

#endif