#include "Devices/SoundChannelSquareWave.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

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

    frequencyTimerPeriod = 0;
}

uint8_t SoundChannelSquareWave::getSoundLength() {
    return soundModeLengthDuty & 0b00111111;
}

uint8_t SoundChannelSquareWave::getSoundDuty() {
    return (soundModeLengthDuty & 0b11000000) >> 6;
}

uint16_t SoundChannelSquareWave::getFrequency() {
    return (uint16_t) soundModeFrequencyLow | (((uint16_t)soundModeFrequencyHigh & 0b111) << 8);
}

float SoundChannelSquareWave::getFrequencyTimerTicks() {
    return frequencyTimerTicks;
}

void SoundChannelSquareWave::addToFrequencyTimerTicks(float ticks) {
    frequencyTimerTicks += ticks;
}

uint8_t SoundChannelSquareWave::getLength() {
    return soundModeLengthDuty & 0b111111;
}

uint8_t SoundChannelSquareWave::getInitialVolume() {
    return (soundModeEnvelope & 0xF0) >> 4;
}

uint8_t SoundChannelSquareWave::getInternalLengthCounter() {
    return internalLengthCounter;
}

uint16_t SoundChannelSquareWave::getFrequencyTimerPeriod() {
    return frequencyTimerPeriod;
}

void SoundChannelSquareWave::decrementInternalLengthCounter(int decrements) {
    internalLengthCounter -= decrements;
    if (internalLengthCounter < 0) internalLengthCounter = 0;
}

void SoundChannelSquareWave::trigger() {
    internalLengthCounter = getLength();
    if (internalLengthCounter == 0) internalLengthCounter = 64;

    uint16_t newFrequencyTimerPeriod = (2048-getFrequency()) * 4;
    if (newFrequencyTimerPeriod != frequencyTimerPeriod) {
        frequencyTimerPeriod = newFrequencyTimerPeriod;
        frequencyTimerTicks = 0;
    }
}

void SoundChannelSquareWave::write8(uint16_t address, uint8_t value) {
    if (address == regSweepAddress) soundModeSweep = value;
    else if (address == regLengthDutyAddress) soundModeLengthDuty = value;
    else if (address == regEnvelopeAddress) soundModeEnvelope = value;
    else if (address == regFreqLowAddress) soundModeFrequencyLow = value;
    else if (address == regFreqHighAddress) {
        soundModeFrequencyHigh = value; 
        if (isBitSet(soundModeFrequencyHigh, 7)) {
            trigger();
        }
    }
}

uint8_t SoundChannelSquareWave::read8(uint16_t address) {
    if (address == regSweepAddress) return soundModeSweep;
    else if (address == regLengthDutyAddress) return soundModeLengthDuty;
    else if (address == regEnvelopeAddress) return soundModeEnvelope;
    else if (address == regFreqLowAddress) return soundModeFrequencyLow;
    else if (address == regFreqHighAddress) return soundModeFrequencyHigh;
    return 0;
}