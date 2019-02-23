
#include "Devices/Audio/SweepCounter.hpp"
#include "Util/LogUtil.hpp"

#define MAX_VOLUME 0xF

#define SWEEP_COUNTER_FREQUENCY_HZ              128.0
#define SWEEP_COUNTER_INCREMENTS_PER_SAMPLE     ((SWEEP_COUNTER_FREQUENCY_HZ / 1000.0) * SAMPLE_INTERVAL_MS)          

void SweepCounter::load(int frequency, bool up, int period, int shifts) {
    this->shadowFrequency = frequency;
    this->up = up;
    this->counterPeriod = period;
    this->counterValue = period;
    this->shifts = shifts;
    this->clockValue = 0.0;
    this->enabled = period != 0 || shifts != 0;
}

bool SweepCounter::isEnabled() {
    return enabled;
}

void SweepCounter::update() {
    clockValue += SWEEP_COUNTER_INCREMENTS_PER_SAMPLE;
    if (clockValue >= 1.0) {
        clockValue -= 1.0;
        counterValue--;
        if (counterValue == 0 && counterPeriod > 0) {
            counterValue = counterPeriod;
            if (overflowCheck() && shifts > 0) {
                int newFrequency = shadowFrequency >> shifts;
                newFrequency *= (up ? 1 : -1);
                shadowFrequency += newFrequency;
                enabled = overflowCheck();
            } else {
                enabled = false;
            }
        }
    }
}

bool SweepCounter::overflowCheck() {
    int newFrequency = shadowFrequency >> shifts;
    newFrequency *= (up ? 1 : -1);
    newFrequency += shadowFrequency;
    return newFrequency < 2047;
}

float SweepCounter::getFrequency() {
    return shadowFrequency;
}