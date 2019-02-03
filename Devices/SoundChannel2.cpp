#include "Devices/SoundChannel2.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

uint8_t SoundChannel2::getSoundLength() {
    return soundModeLengthDuty & 0b00111111;
}

uint8_t SoundChannel2::getSoundDuty() {
    return (soundModeLengthDuty & 0b11000000) >> 6;
}

uint16_t SoundChannel2::getFrequency() {
    return ((uint16_t) read8(NR_23_SOUND_MODE_FREQ_LO)) | 
           (((uint16_t)(read8(NR_24_SOUND_MODE_FREQ_HI) & 0b111)) << 8);
}

uint8_t SoundChannel2::getLength() {
    return read8(NR_21_SOUND_MODE_LENGTH_DUTY) & 0b111111;
}

uint8_t SoundChannel2::getInitialVolume() {
    return (read8(NR_22_SOUND_MODE_ENVELOPE) & 0xF0) >> 4;
}

uint8_t SoundChannel2::getInternalLengthCounter() {
    return internalLengthCounter;
}

uint16_t SoundChannel2::getInternalFrequencyTimerPeriod() {
    return internalFrequencyTimerPeriod;
}

void SoundChannel2::decrementInternalLengthCounter(int decrements) {
    internalLengthCounter -= decrements;
    if (internalLengthCounter < 0) internalLengthCounter = 0;
}

void SoundChannel2::trigger() {
    internalLengthCounter = getLength();
    if (internalLengthCounter == 0) internalLengthCounter = 64;

    internalFrequencyTimerPeriod = (2048-getFrequency()) * 4;
}

void SoundChannel2::write8(uint16_t address, uint8_t value) {
    switch (address) {
        case NR_21_SOUND_MODE_LENGTH_DUTY: {
            soundModeLengthDuty = value; 
            //TRACE_SOUND("Sound len: " << cout8Hex(getSoundLength()) << " duty " << cout8Hex(getSoundDuty()) << endl);
            break;
        }
        case NR_22_SOUND_MODE_ENVELOPE: {
            soundModeEnvelope = value; 
            //TRACE_SOUND("Sound envelope" << endl);
            break;
        }
        case NR_23_SOUND_MODE_FREQ_LO: {
            soundModeFrequencyLow = value; 
            //TRACE_SOUND("Sound freq lo" << endl);
            break;
        }
        case NR_24_SOUND_MODE_FREQ_HI: {
            soundModeFrequencyHigh = value; 
            if (isBitSet(soundModeFrequencyHigh, 7)) {
                trigger();
            }
            break;
        }
    }
}

uint8_t SoundChannel2::read8(uint16_t address) {
    switch (address) {
        case NR_21_SOUND_MODE_LENGTH_DUTY: return soundModeLengthDuty;
        case NR_22_SOUND_MODE_ENVELOPE: return soundModeEnvelope;
        case NR_23_SOUND_MODE_FREQ_LO: return soundModeFrequencyLow;
        case NR_24_SOUND_MODE_FREQ_HI: return soundModeFrequencyHigh;
    }
    return 0;
}