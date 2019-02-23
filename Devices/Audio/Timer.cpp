
#include "Devices/Audio/Timer.hpp"

void Timer::setPeriod(int period) {
    this->period = period;
    this->value = period;
    this->outputClockTicks = 0;
}

void Timer::updatePeriod(int period) {
    this->period = period;
}

long Timer::getOutputClockTicks() {
    return outputClockTicks;
}

bool Timer::update() {
    int newValue = value - MASTER_CLOCK_TICKS_PER_SAMPLE;
    if (newValue <= 0) {
        value = period + newValue;
        outputClockTicks++;
        return true;
    } else {
        value = newValue;
        return false;
    }
}