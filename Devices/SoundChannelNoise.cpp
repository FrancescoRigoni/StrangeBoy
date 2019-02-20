#include "Devices/SoundChannelNoise.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

bool SoundChannelNoise::getLSBOfLFSR() {
    return isBitSet(lfsr, 0);
}

bool SoundChannelNoise::isChannelEnabled() {
    return channelEnabled;
}

void SoundChannelNoise::addToFrequencyTimerTicks(float ticks) {
    frequencyCounter += ticks;

    while (frequencyCounter >= frequencyTimerDivisor) {
        //cout << "frequencyCounter: " << frequencyCounter << " frequencyTimerDivisor: " << frequencyTimerDivisor << endl;
        // One clock has been generated
        frequencyCounter -= frequencyTimerDivisor;

        uint8_t firstBit = lfsr & 0b01;
        uint8_t secondBit = (lfsr & 0b10) >> 1;
        uint8_t result = firstBit ^ secondBit;      

        lfsrOutput = result;

        lfsr >>= 1;

        if (isBitSet(soundModePolyCounter, 3)) {
            if (result) {
                setBit16(6, &lfsr);
                setBit16(15, &lfsr);
            } else {
                resetBit16(6, &lfsr);
                resetBit16(15, &lfsr);
            }

        } else {
            if (result) {
                setBit16(15, &lfsr);
            } else {
                resetBit16(15, &lfsr);
            }
        }
    }
}

float SoundChannelNoise::getFrequencyTimerDivisor() {
    return frequencyTimerDivisor;
}

uint8_t SoundChannelNoise::getLength() {
    return soundModeLength & 0b111111;
}

uint8_t SoundChannelNoise::getEnvelopedVolume() {
    return envelopedVolume;
}

int SoundChannelNoise::getEnvelopeTimerDivisor() {
    return envelopeTimerDivisor;
}

void SoundChannelNoise::addToEnvelopeTimerTicks(float ticks) {
    envelopeCounter += ticks;
    if (envelopeCounter >= 1 && envelopeTimerDivisor > 0) {
        // One clock has been generated
        envelopeCounter = envelopeCounter - 1.0;
        bool amplify = isBitSet(soundModeEnvelope, 3);
        if (amplify && envelopedVolume + 1 <= 15) envelopedVolume++;
        else if (!amplify && envelopedVolume - ticks >= 0) envelopedVolume--;
    }
}

void SoundChannelNoise::addToLengthTimerTicks(float ticks) {
    lengthCounter += ticks;
    if (lengthCounter >= 1 && length > 0) {
        // One clock has been generated
        lengthCounter = lengthCounter - 1.0;
        length--;
        bool shouldStopAfterZeroingLength = isBitSet(soundModeFlags, 6);
        if (length == 0 && shouldStopAfterZeroingLength) {
            channelEnabled = false;
        }
    }
}

void SoundChannelNoise::trigger() {
    channelEnabled = true;

    length = (64-getLength());
    lengthCounter = 0;

    uint8_t shiftClock = (soundModePolyCounter & 0b11110000) >> 4;
    uint16_t dividingRatio = (soundModePolyCounter & 0b111);

    switch(dividingRatio) {
        case 0: frequencyTimerDivisor = 8; break;
        case 1: frequencyTimerDivisor = 16; break;
        case 2: frequencyTimerDivisor = 32; break;
        case 3: frequencyTimerDivisor = 48; break;
        case 4: frequencyTimerDivisor = 64; break;
        case 5: frequencyTimerDivisor = 80; break;
        case 6: frequencyTimerDivisor = 96; break;
        case 7: frequencyTimerDivisor = 112; break;
    }

    //frequencyCounter = 0;

    //frequencyTimerDivisor = dividingRatio * shiftClock;
    //frequencyTimerDivisor = dividingRatio;

    if (isBitSet(soundModePolyCounter, 3)) {
        lfsr = 0xFF;
    } else {
        lfsr = 0xFFFF;
    }

    envelopeTimerDivisor = soundModeEnvelope & 0b111;
    envelopedVolume = (soundModeEnvelope & 0xF0) >> 4;
    envelopeCounter = 0;
}

void SoundChannelNoise::write8(uint16_t address, uint8_t value) {
    if (address == NR_41_SOUND_MODE_LENGTH) {
        soundModeLength = value;
        // Length can be reloaded at any time
        length = 64-getLength();
    }
    else if (address == NR_42_SOUND_MODE_ENVELOPE) {
        soundModeEnvelope = value;
    }
    else if (address == NR_43_SOUND_MODE_POLY_COUNTER) {
        soundModePolyCounter = value;
    }
    else if (address == NR_44_SOUND_MODE_FLAGS) {
        soundModeFlags = value;
        if (isBitSet(soundModeFlags, 7)) {
            trigger();
        }
    }
}

uint8_t SoundChannelNoise::read8(uint16_t address) {
    if (address == NR_41_SOUND_MODE_LENGTH) return soundModeLength;
    else if (address == NR_42_SOUND_MODE_ENVELOPE) return soundModeEnvelope;
    else if (address == NR_43_SOUND_MODE_POLY_COUNTER) return soundModePolyCounter;
    else if (address == NR_44_SOUND_MODE_FLAGS) return soundModeFlags & 0b1000000;
    return 0;
}