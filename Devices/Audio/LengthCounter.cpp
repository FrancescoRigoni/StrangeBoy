
#include "Devices/Audio/LengthCounter.hpp"

#define LENGTH_COUNTER_FREQUENCY_HZ             256.0
#define LENGTH_COUNTER_INCREMENTS_PER_SAMPLE    ((LENGTH_COUNTER_FREQUENCY_HZ / 1000.0) * SAMPLE_INTERVAL_MS)

void LengthCounter::load(int data) {
    counterPeriod = 64-data;
    counterValue = counterPeriod;
    clockValue = 0;
}

void LengthCounter::update() {
    clockValue += LENGTH_COUNTER_INCREMENTS_PER_SAMPLE;
    if (clockValue >= 1.0) {
        // One clock generated
        clockValue -= 1.0;
        counterValue--; 
    }
}

bool LengthCounter::isEnabled() {
    return counterValue > 0;
}