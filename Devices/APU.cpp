
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

APU::APU(SoundChannelSquareWave *soundChannel1, SoundChannelSquareWave *soundChannel2, Sound *sound) {
    this->sound = sound;
    this->soundChannel1 = soundChannel1;
    this->soundChannel2 = soundChannel2;

    lastStepTime = TIME_MS;
}

void APU::generateOneBuffer() {
    long millisecondsSinceLastBuffer = TIME_MS - lastStepTime;
    float internalLengthCounterDecrementsPerMilliseconds = LENGTH_COUNTER_FREQ / 1000.0;
    int internalLengthCounterDecrements = internalLengthCounterDecrementsPerMilliseconds * millisecondsSinceLastBuffer;
    soundChannel1->decrementInternalLengthCounter(internalLengthCounterDecrements);
    soundChannel2->decrementInternalLengthCounter(internalLengthCounterDecrements);

    struct AudioBuffer *channel1Buffer = generateSquareWaveBuffer(soundChannel1, millisecondsSinceLastBuffer);
    struct AudioBuffer *channel2Buffer = generateSquareWaveBuffer(soundChannel2, millisecondsSinceLastBuffer);

    float samplesPerMillisecond = SAMPLE_FREQUENCY / 1000.0;
    float samplesToGenerate = millisecondsSinceLastBuffer * samplesPerMillisecond;
    int samplesInBuffer = samplesToGenerate * 2;

    struct AudioBuffer *finalBuffer;
    finalBuffer = new struct AudioBuffer;
    finalBuffer->size = samplesInBuffer * sizeof(uint16_t);
    finalBuffer->buffer = new uint16_t[samplesInBuffer];

    for (int i = 0; i < samplesInBuffer; i++) {
        uint16_t sampleFrom1 = channel1Buffer != 0 ? channel1Buffer->buffer[i] : 0;
        uint16_t sampleFrom2 = channel2Buffer != 0 ? channel2Buffer->buffer[i] : 0;

        finalBuffer->buffer[i] = (sampleFrom1 + sampleFrom2);
    }

    delete[] channel1Buffer;
    delete[] channel2Buffer;

    sound->pushBuffer(finalBuffer);

    lastStepTime = TIME_MS;
}

struct AudioBuffer *APU::generateSquareWaveBuffer(SoundChannelSquareWave *squareWaveChannel, long bufferDurationMilliseconds) {
    int frequency = squareWaveChannel->getFrequency();
    if (frequency == 0) {
        return 0;
    }

    // A timer generates an output clock every N input clocks, where N is the timer's period.
    int frequencyTimerClocksPerSecond = INPUT_MASTER_CLOCK_HZ / squareWaveChannel->getFrequencyTimerPeriod();
    float frequencyTimerClocksPerMillisecond = frequencyTimerClocksPerSecond / 1000.0;

    // Figure out how many samples to generate
    float samplesPerMillisecond = SAMPLE_FREQUENCY / 1000.0;
    float samplesToGenerate = bufferDurationMilliseconds * samplesPerMillisecond;
    float sampleIntervalMs = bufferDurationMilliseconds / samplesToGenerate;

    struct AudioBuffer *buffer = new struct AudioBuffer;
    buffer->size = samplesToGenerate * BYTES_PER_SAMPLE;
    buffer->buffer = new uint16_t[samplesToGenerate * 2];
    uint16_t *bufferPointer = buffer->buffer;

    int16_t volume = volumeToOutputVolume(squareWaveChannel->getInitialVolume());
    int16_t duty = squareWaveChannel->getSoundDuty();

    float frequencyTimerTicksIncrementPerSample = frequencyTimerClocksPerMillisecond * sampleIntervalMs;

    for (int sample = 0; sample < samplesToGenerate; sample += 1) {
        generateSquareWaveSample(bufferPointer, volume, squareWaveChannel->getFrequencyTimerTicks(), duty);
        squareWaveChannel->addToFrequencyTimerTicks(frequencyTimerTicksIncrementPerSample);
        bufferPointer += 2;
    }

    return buffer;
}

void APU::generateSquareWaveSample(uint16_t *bufferPointer, int volume, float frequencyTimerTicks, int duty) {
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
