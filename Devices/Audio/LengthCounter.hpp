
#include "Devices/Audio/CommonAudio.hpp"

#ifndef _LENGTH_COUNTER_H_
#define _LENGTH_COUNTER_H_

class LengthCounter {
private:
    float clockValue;
    int counterValue;
    int counterPeriod;

public:
    void load(int, int);
    void update();
    bool isEnabled();
};

#endif