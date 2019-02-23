
#include "Devices/Audio/CommonAudio.hpp"

#ifndef _ENVELOPE_COUNTER_H_
#define _ENVELOPE_COUNTER_H_

class EnvelopeCounter {
private:
    float clockValue;
    int counterValue;
    int counterPeriod;
    int volume;
    bool up;

public:
    void load(int initial, bool up, int period);
    void update();
    float getVolume();
};

#endif