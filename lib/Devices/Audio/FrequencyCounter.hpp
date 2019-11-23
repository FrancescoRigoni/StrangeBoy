
#ifndef _FREQUENCYCOUNTER_HPP_
#define _FREQUENCYCOUNTER_HPP_

#include "Devices/Audio/CommonAudio.hpp"

class FrequencyCounter {
private:
    float period = 0;
    float value = 0;
    long outputClockTicks = 0;

public:
    void setPeriod(float);
    void updatePeriod(float);
    bool update();
    long getOutputClockTicks();
};

#endif