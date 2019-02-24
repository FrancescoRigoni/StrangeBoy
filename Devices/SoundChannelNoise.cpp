#include "Devices/SoundChannelNoise.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

#define LENGTH (soundModeLength & 0b111111)
#define LENGTH_ON ((soundModeFlags & 0b1000000) >> 6)
#define ENV_VOLUME ((soundModeEnvelope & 0b11110000) >> 4)
#define ENV_UP ((soundModeEnvelope & 0b1000) >> 3)
#define ENV_PERIOD (soundModeEnvelope & 0b111)

#define SHIFT_CLOCK_FREQUENCY   ((soundModePolyCounter & 0b11110000) >> 4)
#define POLY_COUNTER_STEP       ((soundModePolyCounter & 0b1000) >> 3)
#define DIVIDING_RATIO          (soundModePolyCounter & 0b111)

bool SoundChannelNoise::isChannelEnabled() {
    return channelEnabled;
}

void SoundChannelNoise::checkForTrigger() {
    if (isBitClear(soundModeFlags, 7)) {
        return;
    }

    channelEnabled = true;

    updatePeriod();
    // Set length counter
    lengthCounter.load(64, LENGTH);
    // Set envelope counter
    envelopeCounter.load(ENV_VOLUME, ENV_UP, ENV_PERIOD);

    lfsr16 = 0x7FFF;
    lfsr8 = 0x7F;
}

void SoundChannelNoise::updatePeriod() {
    int dividingRatio = DIVIDING_RATIO;
    int frequencyPeriod;
    switch (dividingRatio) {
        case 0: frequencyPeriod = 8; break;
        case 1: frequencyPeriod = 16; break;
        case 2: frequencyPeriod = 32; break;
        case 3: frequencyPeriod = 48; break;
        case 4: frequencyPeriod = 64; break;
        case 5: frequencyPeriod = 80; break;
        case 6: frequencyPeriod = 96; break;
        case 7: frequencyPeriod = 112; break;
    }

    frequencyPeriod <<= SHIFT_CLOCK_FREQUENCY;
    frequencyTimer.setPeriod(frequencyPeriod);
}

float SoundChannelNoise::sample() {
    if (LENGTH_ON && !lengthCounter.isEnabled()) return 0;

    lengthCounter.update();
    envelopeCounter.update();

    bool updated = frequencyTimer.update();
    if (updated) {
        if (POLY_COUNTER_STEP == 1) {
            int bit0 = lfsr8 & 0b01;
            int bit1 = (lfsr8 & 0b10) >> 1;
            int bitResult = bit0 ^ bit1;
            lfsr8 >>= 1;
            lfsr8 |= (bitResult ? 0x40 : 0x00);

            return envelopeCounter.getVolume() * !bitResult;

        } else {
            int bit0 = lfsr16 & 0b01;
            int bit1 = (lfsr16 & 0b10) >> 1;
            int bitResult = bit0 ^ bit1;
            lfsr16 >>= 1;
            lfsr16 |= (bitResult ? 0x4000 : 0x0000);

            return envelopeCounter.getVolume() * !bitResult;
        }
    } else {
        if (POLY_COUNTER_STEP == 1) {
            int bit0 = lfsr8 & 0b01;
            return envelopeCounter.getVolume() * bit0;
        } else {
            int bit0 = lfsr16 & 0b01;
            return envelopeCounter.getVolume() * bit0;
        }
    }
}

void SoundChannelNoise::write8(uint16_t address, uint8_t value) {
    if (address == NR_41_SOUND_MODE_LENGTH) {
        soundModeLength = value;
        lengthCounter.load(64, LENGTH);
    }
    else if (address == NR_42_SOUND_MODE_ENVELOPE) {
        soundModeEnvelope = value;
    }
    else if (address == NR_43_SOUND_MODE_POLY_COUNTER) {
        soundModePolyCounter = value;
        updatePeriod();
    }
    else if (address == NR_44_SOUND_MODE_FLAGS) {
        soundModeFlags = value;
        checkForTrigger();
    }
}

uint8_t SoundChannelNoise::read8(uint16_t address) {
    if (address == NR_41_SOUND_MODE_LENGTH) return soundModeLength;
    else if (address == NR_42_SOUND_MODE_ENVELOPE) return soundModeEnvelope;
    else if (address == NR_43_SOUND_MODE_POLY_COUNTER) return soundModePolyCounter;
    else if (address == NR_44_SOUND_MODE_FLAGS) return soundModeFlags & 0b1000000;
    return 0;
}