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

bool SoundChannelSquareWave::isChannelEnabled() {
    return channelEnabled;
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

uint8_t SoundChannelSquareWave::getEnvelopedVolume() {
    return envelopedVolume;
}

void SoundChannelSquareWave::addToEnvelopeTimerTicks(float ticks) {
    envelopeCounter += ticks;
    if (envelopeCounter > 1 && internalNumberOfEnvelopeOps > 0) {
        envelopeCounter--;
        bool amplify = isBitSet(soundModeEnvelope, 3);
        internalNumberOfEnvelopeOps--;
        if (amplify && envelopedVolume + 1 <= 15) envelopedVolume++;
        else if (!amplify && envelopedVolume - ticks >= 0) envelopedVolume--;
    }
}

float SoundChannelSquareWave::getInternalLengthCounter() {
    return internalLengthCounter;
}

bool SoundChannelSquareWave::lengthCounterEnabled() {
    return soundModeFrequencyHigh & 0b01000000;
}

uint16_t SoundChannelSquareWave::getFrequencyTimerPeriod() {
    return frequencyTimerPeriod;
}

void SoundChannelSquareWave::decrementInternalLengthCounter(float decrements) {
    internalLengthCounter -= decrements;
    if (internalLengthCounter < 0) internalLengthCounter = 0;
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

int sweepopscount = 0;

void SoundChannelSquareWave::addToSweepTimerTicks(float ticks) {
    sweepTimerTicks += ticks;
    if (sweepTimerTicks > 1) {
        sweepTimerTicks = sweepTimerTicks - 1.0;
        if (internalSweepEnabledFlag && getSweepTimerDivisor()) {
            sweepFrequencyCalculate();
            sweepopscount++;
            //if (sweepopscount > 1) internalSweepEnabledFlag = false;
        }
    }
}

void SoundChannelSquareWave::trigger() {
    channelEnabled = true;

    sweepopscount = 0;

    internalLengthCounter = getLength();
    if (internalLengthCounter == 0) internalLengthCounter = 64;

    uint16_t newFrequencyTimerPeriod = (2048-getFrequency()) * 4;
    if (newFrequencyTimerPeriod != frequencyTimerPeriod) {
        frequencyTimerPeriod = newFrequencyTimerPeriod;
        frequencyTimerTicks = 0;
    }

    internalNumberOfEnvelopeOps = soundModeEnvelope & 0b111;
    envelopedVolume = (soundModeEnvelope & 0xF0) >> 4;
    envelopeCounter = 0;

    sweepShadowFrequency = getFrequency();
    sweepTimerTicks = 0;
    internalSweepEnabledFlag = getSweepTimerDivisor() != 0 || getSweepShifts() != 0;

    if (getSweepShifts() != 0) sweepFrequencyCalculate();
}

void SoundChannelSquareWave::sweepFrequencyCalculate() {
    uint16_t newFrequencyOperand = sweepShadowFrequency >> getSweepShifts();
    uint16_t newFrequency = sweepShadowFrequency;

    if (sweepUp()) newFrequency += newFrequencyOperand;
    else newFrequency -= newFrequencyOperand;

    //cout << "Sweep " << sweepShadowFrequency << " new " <<  newFrequency << endl;

    if (newFrequency > 2047) {
        //cout << "sweep disabling channel!" << endl;
        channelEnabled = false;
        internalSweepEnabledFlag = false;
    } else {
        soundModeFrequencyLow = (uint8_t) newFrequency;
        soundModeFrequencyHigh &= 0b11111000;
        soundModeFrequencyHigh |= (newFrequency >> 8) & 0b111;

        sweepShadowFrequency = getFrequency();
        //cout << "sweepShadowFrequency " << sweepShadowFrequency << endl;

        uint16_t newFrequencyTimerPeriod = (2048-getFrequency()) * 4;
        frequencyTimerPeriod = newFrequencyTimerPeriod;
    }
}

void SoundChannelSquareWave::write8(uint16_t address, uint8_t value) {
    if (address == regSweepAddress) {
        soundModeSweep = value;
        //cout << "Write sweep" << endl;
    }
    else if (address == regLengthDutyAddress) {
        soundModeLengthDuty = value;
        // Length can be reloaded at any time
        internalLengthCounter = getLength();
        //cout << "Write length" << endl;
    }
    else if (address == regEnvelopeAddress) {
        soundModeEnvelope = value;
        internalNumberOfEnvelopeOps = soundModeEnvelope & 0b111;
        envelopedVolume = (soundModeEnvelope & 0xF0) >> 4;
        envelopeCounter = 0;
        //cout << "Write envelope" << endl;
    }
    else if (address == regFreqLowAddress) {
        soundModeFrequencyLow = value;
        //cout << "Write freq lo" << endl;
    }
    else if (address == regFreqHighAddress) {
        //cout << "Write freq hi" << endl;
        soundModeFrequencyHigh = value; 
        if (isBitSet(soundModeFrequencyHigh, 7)) {
            cout << "trigger" << endl;
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