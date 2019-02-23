#include "Devices/SoundChannelSquareWave.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

#define LENGTH_COUNTER_FREQ         256.0
#define ENVELOPE_COUNTER_FREQ        64.0
#define SWEEP_COUNTER_FREQ          128.0

#define FREQUENCY ((uint16_t)soundModeFrequencyLow | ((uint16_t)soundModeFrequencyHigh & 0b111) << 8)
#define FREQUENCY_TO_PERIOD(freq) ((2048-freq)*4)
#define DUTY ((soundModeLengthDuty & 0b11000000) >> 6)
#define LENGTH (soundModeLengthDuty & 0b111111)
#define LENGTH_ON ((soundModeFrequencyHigh & 0b1000000) >> 6)
#define ENV_VOLUME ((soundModeEnvelope & 0b11110000) >> 4)
#define ENV_UP ((soundModeEnvelope & 0b1000) >> 3)
#define ENV_PERIOD (soundModeEnvelope & 0b111)
#define SWEEP_PERIOD ((soundModeSweep & 0b1110000) >> 4)
#define SWEEP_DOWN ((soundModeSweep & 0b1000) >> 3)
#define SWEEP_SHIFTS (soundModeSweep & 0b111)

SoundChannelSquareWave::SoundChannelSquareWave(uint16_t regSweepAddress, 
                                               uint16_t regLengthDutyAddress,
                                               uint16_t regEnvelopeAddress,
                                               uint16_t regFreqLowAddress,
                                               uint16_t regFreqHighAddress) {
    this->regSweepAddress = regSweepAddress;
    this->regLengthDutyAddress = regLengthDutyAddress;
    this->regEnvelopeAddress = regEnvelopeAddress;
    this->regFreqLowAddress = regFreqLowAddress;
    this->regFreqHighAddress = regFreqHighAddress;
}

bool SoundChannelSquareWave::isChannelEnabled() {
    return channelEnabled;
}

void SoundChannelSquareWave::setChannelEnabled(bool enabled) {
    channelEnabled = enabled;
}

void SoundChannelSquareWave::checkForTrigger() { 
    if (isBitClear(soundModeFrequencyHigh, 7)) {
        return;
    }

    channelEnabled = true;

    // Calculate new frequency period
    frequencyTimer.setPeriod(FREQUENCY_TO_PERIOD(FREQUENCY));
    // Set length counter
    lengthCounter.load(LENGTH);
    // Set envelope counter
    envelopeCounter.load(ENV_VOLUME, ENV_UP, ENV_PERIOD);
    // Set sweep counter
    sweepCounter.load(FREQUENCY_TO_PERIOD(FREQUENCY), !SWEEP_DOWN, SWEEP_PERIOD, SWEEP_SHIFTS);
}

float SoundChannelSquareWave::sample() {
    if (sweepCounter.isEnabled()) {
        sweepCounter.update();
        frequencyTimer.updatePeriod(FREQUENCY_TO_PERIOD(sweepCounter.getFrequency()));
    }

    frequencyTimer.update();
    lengthCounter.update();
    envelopeCounter.update();

    if (LENGTH_ON && !lengthCounter.isEnabled()) return 0;

    int frequencyTicksMod8 = frequencyTimer.getOutputClockTicks() % 8;
    int duty = DUTY;
    float volume = envelopeCounter.getVolume();

    switch (frequencyTicksMod8) {
        case 0: return volume;
        case 1: return duty >= 1 ? volume : 0;
        case 2: return duty >= 2 ? volume : 0;
        case 3: return duty >= 2 ? volume : 0;
        case 4: return duty == 3 ? volume : 0;
        case 5: return duty == 3 ? volume : 0;
        case 6: return 0.0;
        case 7: return 0.0;
    }

    return 0;
}

void SoundChannelSquareWave::write8(uint16_t address, uint8_t value) {
    if (address == regSweepAddress) {
        soundModeSweep = value;
    }
    else if (address == regLengthDutyAddress) {
        soundModeLengthDuty = value;
        lengthCounter.load(LENGTH);
    }
    else if (address == regEnvelopeAddress) {
        soundModeEnvelope = value;
        envelopeCounter.load(ENV_VOLUME, ENV_UP, ENV_PERIOD);
    }
    else if (address == regFreqLowAddress) {
        soundModeFrequencyLow = value;
    }
    else if (address == regFreqHighAddress) {
        soundModeFrequencyHigh = value;
        checkForTrigger();
    }
}

uint8_t SoundChannelSquareWave::read8(uint16_t address) {
    if (address == regSweepAddress) return soundModeSweep;
    else if (address == regLengthDutyAddress) return soundModeLengthDuty;
    else if (address == regEnvelopeAddress) return soundModeEnvelope;
    else if (address == regFreqLowAddress) return soundModeFrequencyLow;
    else if (address == regFreqHighAddress) {
        // Only bit 6 can be read
        return soundModeFrequencyHigh & 0b01000000;
    }
    return 0;
}
