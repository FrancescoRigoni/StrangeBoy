#include "Devices/SoundChannelSquareWave.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

SoundChannelSquareWave::SoundChannelSquareWave(int channelNumber,
                                               uint16_t regSweepAddress, 
                                               uint16_t regLengthDutyAddress,
                                               uint16_t regEnvelopeAddress,
                                               uint16_t regFreqLowAddress,
                                               uint16_t regFreqHighAddress) {
    this->channelNumber = channelNumber;
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

uint8_t SoundChannelSquareWave::getSoundDuty() {
    return (soundModeLengthDuty & 0b11000000) >> 6;
}

uint16_t SoundChannelSquareWave::getFrequency() {
    return (uint16_t) soundModeFrequencyLow | (((uint16_t)soundModeFrequencyHigh & 0b111) << 8);
}

float SoundChannelSquareWave::getFrequencyTimerTicks() {
    return frequencyCounter;
}

void SoundChannelSquareWave::addToFrequencyTimerTicks(float ticks) {
    frequencyCounter += ticks;
}

uint16_t SoundChannelSquareWave::getFrequencyTimerDivisor() {
    return frequencyTimerDivisor;
}

uint8_t SoundChannelSquareWave::getLength() {
    return soundModeLengthDuty & 0b111111;
}

uint8_t SoundChannelSquareWave::getEnvelopedVolume() {
    return envelopedVolume;
}

int SoundChannelSquareWave::getEnvelopeTimerDivisor() {
    return envelopeTimerDivisor;
}

int SoundChannelSquareWave::getSweepTimerDivisor() {
    return (soundModeSweep & 0b1110000) >> 4;
}

bool SoundChannelSquareWave::sweepUp() {
    return isBitClear(soundModeSweep, 3);
}

int SoundChannelSquareWave::getSweepShifts() {
    return soundModeSweep & 0b111;
}

void SoundChannelSquareWave::addToEnvelopeTimerTicks(float ticks) {
    envelopeCounter += ticks;
    if (envelopeCounter >= 1 && envelopeTimerDivisor > 0) {
        // One clock has been generated
        envelopeCounter = envelopeCounter - 1.0;
        bool amplify = isBitSet(soundModeEnvelope, 3);
        if (amplify && envelopedVolume + 1 <= 15) envelopedVolume++;
        else if (!amplify && envelopedVolume - ticks >= 0) envelopedVolume--;
    }
}

void SoundChannelSquareWave::addToLengthTimerTicks(float ticks) {
    lengthCounter += ticks;
    if (lengthCounter >= 1 && length > 0) {
        // One clock has been generated
        lengthCounter = lengthCounter - 1.0;
        length--;
        bool shouldStopAfterZeroingLength = isBitSet(soundModeFrequencyHigh, 6);
        if (length == 0 && shouldStopAfterZeroingLength) {
            channelEnabled = false;
        }
    }
}

void SoundChannelSquareWave::addToSweepTimerTicks(float ticks) {
    sweepCounter += ticks;
    if (sweepCounter >= 1) {
        // One clock has been generated
        sweepCounter = sweepCounter - 1.0;
        if (sweepEnabled && getSweepTimerDivisor()) {
            sweepFrequencyCalculate();
        }
    }
}

void SoundChannelSquareWave::trigger() {
    channelEnabled = true;

    length = (64-getLength());
    lengthCounter = 0;

    uint16_t newFrequencyTimerDivisor = (2048-getFrequency()) * 4;
    if (newFrequencyTimerDivisor != frequencyTimerDivisor) {
        frequencyTimerDivisor = newFrequencyTimerDivisor;
        frequencyCounter = 0;
    }

    envelopeTimerDivisor = soundModeEnvelope & 0b111;
    envelopedVolume = (soundModeEnvelope & 0xF0) >> 4;
    envelopeCounter = 0;

    sweepShadowFrequency = getFrequency();
    sweepCounter = 0;
    sweepEnabled = getSweepTimerDivisor() != 0 || getSweepShifts() != 0;

    if (getSweepShifts() != 0) sweepFrequencyCalculate();
}

void SoundChannelSquareWave::sweepFrequencyCalculate() {
    uint16_t newFrequencyOperand = sweepShadowFrequency >> getSweepShifts();
    uint16_t newFrequency = sweepShadowFrequency;

    if (sweepUp()) newFrequency += newFrequencyOperand;
    else newFrequency -= newFrequencyOperand;

    if (newFrequency > 2047) {
        channelEnabled = false;
        sweepEnabled = false;

    } else {
        soundModeFrequencyLow = (uint8_t) newFrequency;
        soundModeFrequencyHigh &= 0b11111000;
        soundModeFrequencyHigh |= (newFrequency >> 8) & 0b111;

        sweepShadowFrequency = getFrequency();
        uint16_t newFrequencyTimerDivisor = (2048-getFrequency()) * 4;
        frequencyTimerDivisor = newFrequencyTimerDivisor;
    }
}

void SoundChannelSquareWave::write8(uint16_t address, uint8_t value) {
    if (address == regSweepAddress) {
        soundModeSweep = value;
    }
    else if (address == regLengthDutyAddress) {
        soundModeLengthDuty = value;
        // Length can be reloaded at any time
        length = 64-getLength();
    }
    else if (address == regEnvelopeAddress) {
        soundModeEnvelope = value;
        envelopeTimerDivisor = soundModeEnvelope & 0b111;
        envelopedVolume = (soundModeEnvelope & 0xF0) >> 4;
        envelopeCounter = 0;
    }
    else if (address == regFreqLowAddress) {
        soundModeFrequencyLow = value;
    }
    else if (address == regFreqHighAddress) {
        soundModeFrequencyHigh = value;
        if (isBitSet(soundModeFrequencyHigh, 7)) {
            trigger();
        }
    }
}

uint8_t SoundChannelSquareWave::read8(uint16_t address) {
    if (address == regSweepAddress) return soundModeSweep;
    else if (address == regLengthDutyAddress) {
        return soundModeLengthDuty;
    }
    else if (address == regEnvelopeAddress) return soundModeEnvelope;
    else if (address == regFreqLowAddress) return soundModeFrequencyLow;
    else if (address == regFreqHighAddress) {
        // Only bit 6 can be read
        return soundModeFrequencyHigh & 0b01000000;
    }
    return 0;
}