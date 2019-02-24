
#ifndef _FREQUENCYCOUNTER_HPP_
#define _FREQUENCYCOUNTER_HPP_

#include "Devices/Audio/CommonAudio.hpp"

class FrequencyCounter {
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