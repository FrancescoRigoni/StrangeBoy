#include "Devices/SoundChannelWave.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"

#define FREQUENCY (((uint16_t)soundRegFrequencyLow) | (((uint16_t)soundRegFrequencyHigh) & 0b111) << 8)
#define FREQUENCY_TO_PERIOD(freq) ((2048-freq)*2)
#define OUTPUT_LEVEL ((soundRegOutputLevel & 0b1100000) >> 5)
#define LENGTH_ON ((soundRegFrequencyHigh & 0b1000000) >> 6)

bool SoundChannelWave::isChannelEnabled() {
    return channelEnabled && isBitSet(soundRegOnOff, 7);
}

float SoundChannelWave::sample() {
    lengthCounter.update();
    if (LENGTH_ON && !lengthCounter.isEnabled()) return 0;

    frequencyTimer.update();

    uint8_t outputValue;

    int frequencyTicksMod32 = frequencyTimer.getOutputClockTicks() % 32;
    int position = frequencyTicksMod32 / 2;
    int positionInByte = frequencyTicksMod32 % 2;

    if (positionInByte == 0) outputValue = highNibbleOf(waveRam[position]);
    else outputValue = lowNibbleOf(waveRam[position]);

    int outputLevel = OUTPUT_LEVEL;
    switch(outputLevel) {
        case 0: outputValue >>= 4; break;
        case 2: outputValue >>= 1; break;
        case 3: outputValue >>= 2; break;
    }

    return ((float)outputValue)/16.0;
}

void SoundChannelWave::checkForTrigger() {
    if (isBitClear(soundRegFrequencyHigh, 7)) {
        return;
    }

    channelEnabled = true;
    lengthCounter.load(256, soundRegLength);

    frequencyTimer.setPeriod(FREQUENCY_TO_PERIOD(FREQUENCY));

    position = 0;
}

void SoundChannelWave::updatePeriod() {
    frequencyTimer.updatePeriod(FREQUENCY_TO_PERIOD(FREQUENCY));
}

void SoundChannelWave::write8(uint16_t address, uint8_t value) {
    if (address == NR_30_SOUND_ON_OFF) {
        soundRegOnOff = value;

    } else if (address == NR_31_SOUND_LENGTH) {
        soundRegLength = value;
        lengthCounter.load(256, soundRegLength);

    } else if (address == NR_32_SOUND_OUTPUT_LEVEL) {
        soundRegOutputLevel = value;

    } else if (address == NR_33_SOUND_MODE_FREQ_LO) {
        soundRegFrequencyLow = value;
        updatePeriod();

    } else if (address == NR_34_SOUND_MODE_FREQ_HI) {
        soundRegFrequencyHigh = value;
        updatePeriod();
        checkForTrigger();

    } else if (address >= WAVE_RAM_START && address <= WAVE_RAM_END) {
        waveRam[address-WAVE_RAM_START] = value;
    }
}

uint8_t SoundChannelWave::read8(uint16_t address) {
    if (address == NR_30_SOUND_ON_OFF) {
        // Only bit 7 can be read
        return soundRegOnOff & 0b10000000;

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