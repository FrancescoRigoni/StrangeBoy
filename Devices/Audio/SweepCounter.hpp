
#include "Devices/Audio/CommonAudio.hpp"

#ifndef _SWEEP_COUNTER_H_
#define _SWEEP_COUNTER_H_

class SweepCounter {
private:
    float clockValue;
    int counterValue;
    int counterPeriod;
    int shadowFrequency;
    int shifts;
    bool up;
    bool enabled;

    bool overflowCheck();
    void frequencyCalculate();

public:
    void load(int frequency, bool up, int period, int shifts);
    bool update();
    int getFrequency();
    bool isEnabled();
};

#endif