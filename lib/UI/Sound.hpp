
#ifndef _SOUND_HPP_
#define _SOUND_HPP_

#include <cstdint>

struct AudioBuffer {
    uint16_t size;
    uint16_t *buffer;
};

class Sound {
public:
    virtual void init();
    virtual void run();
    virtual void pushBuffer(struct AudioBuffer *);
};

#endif