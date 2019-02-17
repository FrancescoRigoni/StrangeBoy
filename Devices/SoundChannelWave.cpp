#include "Devices/SoundChannelWave.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

bool SoundChannelWave::isChannelEnabled() {
    return channelEnabled && isBitSet(soundRegOnOff, 7);
}

uint8_t SoundChannelWave::getOutputLevel() {
    return (soundRegOutputLevel & 0b1100000) >> 5;
}

uint16_t SoundChannelWave::getFrequency() {
    return (uint16_t) soundRegFrequencyLow | (((uint16_t)soundRegFrequencyHigh & 0b111) << 8);
}

float SoundChannelWave::getFrequencyTimerTicks() {
    return frequencyCounter;
}

void SoundChannelWave::addToFrequencyTimerTicks(float ticks) {
    frequencyCounter += ticks;
    if (frequencyCounter > 1.0) {
        // One clock has been generated
        frequencyCounter -= 1.0;
        position++;
        position %= 32;

        bool highNibble = (position % 2) == 0;
        if (highNibble) sampleBuffer = highNibbleOf(waveRam[position]);
        else sampleBuffer = lowNibbleOf(waveRam[position]);
    }
}

uint8_t SoundChannelWave::readCurrentSample() {
    return sampleBuffer;
}

uint16_t SoundChannelWave::getFrequencyTimerDivisor() {
    return frequencyTimerDivisor;
}

uint8_t SoundChannelWave::getLength() {
    return soundRegLength;
}

void SoundChannelWave::addToLengthTimerTicks(float ticks) {
    lengthCounter += ticks;
    if (lengthCounter >= 1 && length > 0) {
        // One clock has been generated
        lengthCounter = lengthCounter - 1.0;
        length--;
        bool shouldStopAfterZeroingLength = isBitSet(soundRegFrequencyHigh, 6);
        if (length == 0 && shouldStopAfterZeroingLength) {
            channelEnabled = false;
        }
    }
}

void SoundChannelWave::trigger() {
    length = (256-getLength());
    lengthCounter = 0;

    uint16_t newFrequencyTimerDivisor = (2048-getFrequency()) * 2;
    if (newFrequencyTimerDivisor != frequencyTimerDivisor) {
        frequencyTimerDivisor = newFrequencyTimerDivisor;
        frequencyCounter = 0;
    }

    position = 0;
    channelEnabled = true;
}

void SoundChannelWave::write8(uint16_t address, uint8_t value) {
    if (address == NR_30_SOUND_ON_OFF) {
        soundRegOnOff = value;

    } else if (address == NR_31_SOUND_LENGTH) {
        soundRegLength = value;
        // Length can be reloaded at any time
        length = 256-getLength();

    } else if (address == NR_32_SOUND_OUTPUT_LEVEL) {
        soundRegOutputLevel = value;

    } else if (address == NR_33_SOUND_MODE_FREQ_LO) {
        soundRegFrequencyLow = value;

    } else if (address == NR_34_SOUND_MODE_FREQ_HI) {
        soundRegFrequencyHigh = value;
        if (isBitSet(soundRegFrequencyHigh, 7)) {
            trigger();
        }

    } else if (address >= WAVE_RAM_START && address <= WAVE_RAM_END) {
        waveRam[address-WAVE_RAM_START] = value;
    }
}

uint8_t SoundChannelWave::read8(uint16_t address) {
    if (address == NR_30_SOUND_ON_OFF) {
        // Only bit 7 can be read
        return soundRegOnOff;

    } else if (address == NR_31_SOUND_LENGTH) {
        return soundRegLength;

    } else if (address == NR_32_SOUND_OUTPUT_LEVEL) {
        // Only bits 6-5 can be read
        return soundRegOutputLevel & 0b1100000;

    } else if (address == NR_33_SOUND_MODE_FREQ_LO) {
        return soundRegFrequencyLow;

    } else if (address == NR_34_SOUND_MODE_FREQ_HI) {
        // Only bit 6 can be read
        return soundRegFrequencyHigh & 0b01000000;

    } else if (address >= WAVE_RAM_START && address <= WAVE_RAM_END) {
        return waveRam[address-WAVE_RAM_START];
    }
    return 0;
}