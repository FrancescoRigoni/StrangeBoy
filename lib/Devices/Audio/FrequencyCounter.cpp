
#include "Devices/Audio/FrequencyCounter.hpp"
#include "Util/LogUtil.hpp"

void FrequencyCounter::setPeriod(float period) {
    this->period = period;
    this->value = period;
    this->outputClockTicks = 0;
}

void FrequencyCounter::updatePeriod(float period) {
    this->period = period;
}

long FrequencyCounter::getOutputClockTicks() {
    return outputClockTicks;
}

bool FrequencyCounter::update() {
    
    
    float newValue = value - MASTER_CLOCK_TICKS_PER_SAMPLE;
    if (newValue <= 0) {
        value = period + newValue;
        outputClockTicks++;
        return true;
    } else {
        value = newValue;
        return false;
    }
}