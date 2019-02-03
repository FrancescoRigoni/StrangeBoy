#include "Devices/SoundChannel1.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

uint8_t SoundChannel1::getSoundLength() {
    return soundModeLengthDuty & 0b00111111;
}

uint8_t SoundChannel1::getSoundDuty() {
    return (soundModeLengthDuty & 0b11000000) >> 6;
}

uint16_t SoundChannel1::getFrequency() {
    return ((uint16_t) read8(NR_13_SOUND_MODE_FREQ_LO)) | 
           (((uint16_t)(read8(NR_14_SOUND_MODE_FREQ_HI) & 0b111)) << 8);
}

uint8_t SoundChannel1::getLength() {
    return read8(NR_11_SOUND_MODE_LENGTH_DUTY) & 0b111111;
}

uint8_t SoundChannel1::getInitialVolume() {
    return (read8(NR_12_SOUND_MODE_ENVELOPE) & 0xF0) >> 4;
}

uint8_t SoundChannel1::getInternalLengthCounter() {
    return internalLengthCounter;
}

uint16_t SoundChannel1::getInternalFrequencyTimerPeriod() {
    return internalFrequencyTimerPeriod;
}

void SoundChannel1::decrementInternalLengthCounter(int decrements) {
    internalLengthCounter -= decrements;
    if (internalLengthCounter < 0) internalLengthCounter = 0;
}

void SoundChannel1::trigger() {
    internalLengthCounter = getLength();
    if (internalLengthCounter == 0) internalLengthCounter = 64;

    internalFrequencyTimerPeriod = (2048-getFrequency()) * 4;
}

void SoundChannel1::write8(uint16_t address, uint8_t value) {
    switch (address) {
        case NR_10_SOUND_MODE_SWEEP: {
            soundModeSweep = value; 
            //TRACE_SOUND("Sound sweep" << endl);
            break;
        }
        case NR_11_SOUND_MODE_LENGTH_DUTY: {
            soundModeLengthDuty = value; 
            //TRACE_SOUND("Sound len: " << cout8Hex(getSoundLength()) << " duty " << cout8Hex(getSoundDuty()) << endl);
            break;
        }
        case NR_12_SOUND_MODE_ENVELOPE: {
            soundModeEnvelope = value; 
            //TRACE_SOUND("Sound envelope" << endl);
            break;
        }
        case NR_13_SOUND_MODE_FREQ_LO: {
            soundModeFrequencyLow = value; 
            //TRACE_SOUND("Sound freq lo" << endl);
            break;
        }
        case NR_14_SOUND_MODE_FREQ_HI: {
            soundModeFrequencyHigh = value; 
            if (isBitSet(soundModeFrequencyHigh, 7)) {
                trigger();
            }
            break;
        }
    }
}

uint8_t SoundChannel1::read8(uint16_t address) {
    switch (address) {
        case NR_10_SOUND_MODE_SWEEP: return soundModeSweep;
        case NR_11_SOUND_MODE_LENGTH_DUTY: return soundModeLengthDuty;
        case NR_12_SOUND_MODE_ENVELOPE: return soundModeEnvelope;
        case NR_13_SOUND_MODE_FREQ_LO: return soundModeFrequencyLow;
        case NR_14_SOUND_MODE_FREQ_HI: return soundModeFrequencyHigh;
    }
    return 0;
}