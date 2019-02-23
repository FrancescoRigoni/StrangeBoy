
#include "Devices/Audio/EnvelopeCounter.hpp"
#include "Util/LogUtil.hpp"

#define MAX_VOLUME 0xF

#define ENVELOPE_COUNTER_FREQUENCY_HZ           64.0
#define ENVELOPE_COUNTER_INCREMENTS_PER_SAMPLE  ((ENVELOPE_COUNTER_FREQUENCY_HZ / 1000.0) * SAMPLE_INTERVAL_MS)          

void EnvelopeCounter::load(int initial, bool up, int period) {
    this->volume = initial;
    this->up = up;
    this->counterPeriod = period;
    this->counterValue = period;
    this->clockValue = 0;
}

void EnvelopeCounter::update() {
    if (volume == 0 && !up) return;
    if (volume == 15 && up) return;
    if (counterPeriod == 0) return;

    clockValue += ENVELOPE_COUNTER_INCREMENTS_PER_SAMPLE;
    if (clockValue >= 1) {
        // One clock generated
        clockValue -= 1.0;
        counterValue--;
        if (counterValue == 0) {
            // Counter reached zero
            counterValue = counterPeriod;
            volume += (up ? 1 : -1);
        }
    }
}

float EnvelopeCounter::getVolume() {
    return (float)volume/(float)MAX_VOLUME;
}