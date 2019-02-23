
#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include "Devices/Audio/CommonAudio.hpp"

class Timer {
private:
    int period = 0;
    int value = 0;
    long outputClockTicks = 0;

public:
    void setPeriod(int);
    void updatePeriod(int);
    bool update();
    long getOutputClockTicks();
};

#endif