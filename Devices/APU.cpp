
#include <cstdint>

#include "Devices/APU.hpp"
#include "Util/ByteUtil.hpp"
#include "Util/LogUtil.hpp"

#define NR_52_ALL_ON_OFF_BIT    7
#define NR_52_4_ON_OFF_BIT      3
#define NR_52_3_ON_OFF_BIT      2
#define NR_52_2_ON_OFF_BIT      1
#define NR_52_1_ON_OFF_BIT      0

#define INPUT_MASTER_CLOCK_HZ   4194304

#define SAMPLE_FREQUENCY        48000.0
#define BYTES_PER_SAMPLE        (sizeof(int16_t) * 2)

#define FRAME_SEQUENCER_FREQ    512
#define LENGTH_COUNTER_FREQ     256.0

#define TIME_MS                                                         \
    chrono::duration_cast<chrono::milliseconds>                         \
        (std::chrono::system_clock::now().time_since_epoch()).count()

using namespace std;
using namespace std::chrono;

APU::APU(SoundChannel1 *soundChannel1, SoundChannel2 *soundChannel2, Sound *sound) {
    this->sound = sound;
    this->soundChannel1 = soundChannel1;
    this->soundChannel2 = soundChannel2;

    lastStepTime = TIME_MS;
}

float frequencyTimer1Ticks = 0;
float frequencyTimer2Ticks = 0;

int internalFrequencyTimer1Period = 0;
int internalFrequencyTimer2Period = 0;

struct AudioBuffer * APU::generateChannel1(long timeSinceLastStep) {
    int frequency = soundChannel1->getFrequency();
    if (frequency == 0) {
        return 0;
    }
    if (soundChannel1->getInternalLengthCounter() == 0) {
        return 0;
    }

    //if (isBitClear(soundChannel1->read8(NR_14_SOUND_MODE_FREQ_HI), 7)) return 0;

    // A timer generates an output clock every N input clocks, where N is the timer's period.
    // This is the wave timer's frequency in Hz.
    if (internalFrequencyTimer1Period != soundChannel1->getInternalFrequencyTimerPeriod()) {
        frequencyTimer1Ticks = 0;
        internalFrequencyTimer1Period = soundChannel1->getInternalFrequencyTimerPeriod();
    }

    int frequencyTimerClocksPerSecond = INPUT_MASTER_CLOCK_HZ / internalFrequencyTimer1Period;
    float frequencyTimerClocksPerMillisecond = frequencyTimerClocksPerSecond / 1000.0;

    // Figure out how many samples to generate
    float samplesPerMillisecond = SAMPLE_FREQUENCY / 1000.0;
    float samplesToGenerate = timeSinceLastStep * samplesPerMillisecond;
    float sampleIntervalMs = timeSinceLastStep / samplesToGenerate;

    struct AudioBuffer *buffer = new struct AudioBuffer;
    buffer->size = samplesToGenerate * BYTES_PER_SAMPLE;
    buffer->buffer = new uint16_t[samplesToGenerate * 2];
    uint16_t *bufferPointer = buffer->buffer;

    int16_t volume = volumeToOutputVolume(soundChannel1->getInitialVolume());

    for (int sample = 0; sample < samplesToGenerate; sample += 1) {
        frequencyTimer1Ticks += frequencyTimerClocksPerMillisecond * sampleIntervalMs;
        generateSquareWave(bufferPointer, volume, frequencyTimer1Ticks, soundChannel1->getSoundDuty());
        bufferPointer += 2;
    }

    return buffer;
}

struct AudioBuffer * APU::generateChannel2(long timeSinceLastStep) {
    int frequency = soundChannel2->getFrequency();
    if (frequency == 0) {
        return 0;
    }
    if (soundChannel2->getInternalLengthCounter() == 0) {
        return 0;
    }

    //if (isBitClear(soundChannel2->read8(NR_24_SOUND_MODE_FREQ_HI), 7)) return 0;

    // A timer generates an output clock every N input clocks, where N is the timer's period.
    // This is the wave timer's frequency in Hz.
    if (internalFrequencyTimer2Period != soundChannel2->getInternalFrequencyTimerPeriod()) {
        frequencyTimer2Ticks = 0;
        internalFrequencyTimer2Period = soundChannel2->getInternalFrequencyTimerPeriod();
    }

    int frequencyTimerClocksPerSecond = INPUT_MASTER_CLOCK_HZ / soundChannel2->getInternalFrequencyTimerPeriod();
    float frequencyTimerClocksPerMillisecond = frequencyTimerClocksPerSecond / 1000.0;

    // Figure out how many samples to generate
    float samplesPerMillisecond = SAMPLE_FREQUENCY / 1000.0;
    float samplesToGenerate = timeSinceLastStep * samplesPerMillisecond;
    float sampleIntervalMs = timeSinceLastStep / samplesToGenerate;

    struct AudioBuffer *buffer = new struct AudioBuffer;
    buffer->size = samplesToGenerate * BYTES_PER_SAMPLE;
    buffer->buffer = new uint16_t[samplesToGenerate * 2];
    uint16_t *bufferPointer = buffer->buffer;

    int16_t volume = volumeToOutputVolume(soundChannel2->getInitialVolume());

    for (int sample = 0; sample < samplesToGenerate; sample += 1) {
        frequencyTimer2Ticks += frequencyTimerClocksPerMillisecond * sampleIntervalMs;
        generateSquareWave(bufferPointer, volume, frequencyTimer2Ticks, soundChannel2->getSoundDuty());
        bufferPointer += 2;
    }

    return buffer;
}

void APU::generateSquareWave(uint16_t *bufferPointer, int volume, float frequencyTimerTicks, int duty) {
    int frequencyTimerTicksModulo8 = (int)frequencyTimerTicks % 8;

    switch (duty) {
        case 0:
            switch (frequencyTimerTicksModulo8) {
                case 0: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 1: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 2: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 3: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 4: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 5: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 6: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 7: *bufferPointer++ = volume; *bufferPointer++ = volume; break;
            }
        break;
        case 1:
            switch (frequencyTimerTicksModulo8) {
                case 0: *bufferPointer++ = volume; *bufferPointer++ = volume; break;
                case 1: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 2: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 3: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 4: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 5: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 6: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 7: *bufferPointer++ = volume; *bufferPointer++ = volume; break;
            }
        break;
        case 2:
            switch (frequencyTimerTicksModulo8) {
                case 0: *bufferPointer++ = volume; *bufferPointer++ = volume; break;
                case 1: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 2: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 3: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 4: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
                case 5: *bufferPointer++ = volume; *bufferPointer++ = volume; break;
                case 6: *bufferPointer++ = volume; *bufferPointer++ = volume; break;
                case 7: *bufferPointer++ = volume; *bufferPointer++ = volume; break;
            }
        break;
        case 3:
            switch (frequencyTimerTicksModulo8) {
                case 0: *bufferPointer++ =      0; *bufferPointer++ =           0; break;
                case 1: *bufferPointer++ = volume; *bufferPointer++ =      volume; break;
                case 2: *bufferPointer++ = volume; *bufferPointer++ =      volume; break;
                case 3: *bufferPointer++ = volume; *bufferPointer++ =      volume; break;
                case 4: *bufferPointer++ = volume; *bufferPointer++ =      volume; break;
                case 5: *bufferPointer++ = volume; *bufferPointer++ =      volume; break;
                case 6: *bufferPointer++ = volume; *bufferPointer++ =      volume; break;
                case 7: *bufferPointer++ =      0; *bufferPointer++ =           0; break;
            }
        break;
    }
}

void APU::step() {

    long millisecondsSinceLastStep = TIME_MS - lastStepTime;
    float internalLengthCounterDecrementsPerMilliseconds = LENGTH_COUNTER_FREQ / 1000.0;
    int internalLengthCounterDecrements = internalLengthCounterDecrementsPerMilliseconds * millisecondsSinceLastStep;
    soundChannel1->decrementInternalLengthCounter(internalLengthCounterDecrements);
    soundChannel2->decrementInternalLengthCounter(internalLengthCounterDecrements);

    struct AudioBuffer *channel1Buffer = generateChannel1(millisecondsSinceLastStep);
    struct AudioBuffer *channel2Buffer = generateChannel2(millisecondsSinceLastStep);

    if (channel1Buffer != 0 && channel2Buffer != 0) {

        struct AudioBuffer *finalBuffer;
        finalBuffer = new struct AudioBuffer;
        finalBuffer->size = channel1Buffer->size;
        finalBuffer->buffer = new uint16_t[channel1Buffer->size / 2];

        for (int i = 0; i < (channel1Buffer->size / sizeof(uint16_t)); i++) {
            uint16_t sampleFrom1 = channel1Buffer->buffer[i];
            uint16_t sampleFrom2 = channel2Buffer->buffer[i];

            //finalBuffer->buffer[i] = (sampleFrom1 + sampleFrom2) / 2;
            if (sampleFrom1 > sampleFrom2) finalBuffer->buffer[i] = sampleFrom1;
            else finalBuffer->buffer[i] = sampleFrom2;

        }

        sound->pushBuffer(finalBuffer);

    } else if (channel1Buffer != 0) {
        sound->pushBuffer(channel1Buffer);
    } else if (channel2Buffer != 0) {
        sound->pushBuffer(channel2Buffer);
    }

    lastStepTime = TIME_MS;

}

uint16_t APU::volumeToOutputVolume(uint16_t volume) {
    float volumeRatio = (float) volume / 16.0f;
    return 1000 * volumeRatio;
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
