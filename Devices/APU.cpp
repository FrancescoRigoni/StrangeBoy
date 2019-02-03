
#include <cstdint>

#include "Devices/APU.hpp"
#include "Util/ByteUtil.hpp"
#include "Util/LogUtil.hpp"

#define NR_52_ALL_ON_OFF_BIT    7
#define NR_52_4_ON_OFF_BIT      3
#define NR_52_3_ON_OFF_BIT      2
#define NR_52_2_ON_OFF_BIT      1
#define NR_52_1_ON_OFF_BIT      0

#define SAMPLE_FREQUENCY        48000
#define BYTES_PER_SAMPLE        (sizeof(int16_t) * 2)
#define SAMPLE_COUNT            800
#define BYTES_PER_BUFFER        (SAMPLE_COUNT * BYTES_PER_SAMPLE)

APU::APU(SoundChannel1 *soundChannel1, Sound *sound) {
    this->sound = sound;
    this->soundChannel1 = soundChannel1;
}

struct AudioBuffer * APU::generateChannel1() {
    uint8_t channel1NR14 = soundChannel1->read8(NR_14_SOUND_MODE_FREQ_HI);
    if (isBitClear(channel1NR14, 7)) {
        // Channel is off
        return 0;
    }

    int frequency = soundChannel1->getFrequency();
    if (frequency == 0) {
        return 0;
    }

    frequency = 131072/(2048-frequency);

    struct AudioBuffer *buffer = new struct AudioBuffer;
    buffer->size = BYTES_PER_BUFFER;
    buffer->buffer = new uint16_t[buffer->size/sizeof(uint16_t)];

    int16_t volume = volumeToOutputVolume(soundChannel1->getInitialVolume());

    int SquareWavePeriod = SAMPLE_FREQUENCY / frequency;
    int HalfSquareWavePeriod = SquareWavePeriod / 2;

    uint16_t *bufferPointer = buffer->buffer;
    for(int SampleIndex = 0; SampleIndex < SAMPLE_COUNT; ++SampleIndex) {
        int16_t SampleValue = ((SampleIndex / HalfSquareWavePeriod) % 2) ? volume : 0;
        *bufferPointer++ = SampleValue;
        *bufferPointer++ = SampleValue;
    }

    return buffer;
}

void APU::step() {

    struct AudioBuffer *channel1Buffer = generateChannel1();
    if (channel1Buffer != 0) {
        TRACE_SOUND("Pushing buffer" << endl);
        sound->pushBuffer(channel1Buffer);
    }

}

uint16_t APU::volumeToOutputVolume(uint16_t volume) {
    float volumeRatio = (float) volume / 16.0f;
    return 3000 * volumeRatio;
}

void APU::write8(uint16_t address, uint8_t value) {
    switch (address) {
        case NR_50_CHANNEL_CONTROL: channelControl = value; break;
        case NR_51_OUTPUT_SELECTION: outputSelection = value; break;
        case NR_52_SOUND_ON_OFF: soundOnOff = value; break;
    }
}

uint8_t APU::read8(uint16_t address) {
    switch (address) {
        case NR_50_CHANNEL_CONTROL: return channelControl;
        case NR_51_OUTPUT_SELECTION: return outputSelection;
        case NR_52_SOUND_ON_OFF: return soundOnOff;
    }
    return 0;
}
