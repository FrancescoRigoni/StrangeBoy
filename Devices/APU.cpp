
#include <cstdint>

#include "Devices/APU.hpp"
#include "Util/ByteUtil.hpp"
#include "Util/LogUtil.hpp"

#define NR_52_ALL_ON_OFF_BIT    7
#define NR_52_4_ON_OFF_BIT      3
#define NR_52_3_ON_OFF_BIT      2
#define NR_52_2_ON_OFF_BIT      1
#define NR_52_1_ON_OFF_BIT      0

#define INPUT_MASTER_CLOCK_HZ   4194304.0

#define SAMPLE_FREQUENCY        48000.0
#define BYTES_PER_SAMPLE        (sizeof(int16_t) * 2)

#define LENGTH_COUNTER_FREQ         256.0
#define ENVELOPE_COUNTER_FREQ        64.0
#define SWEEP_COUNTER_FREQ          128.0

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

    //if (isBitClear(soundOnOff, NR_52_ALL_ON_OFF_BIT)) {
        long millisecondsSinceLastBuffer = (TIME_MS - lastStepTime);

        struct AudioBuffer *channel1Buffer = 0;
        struct AudioBuffer *channel2Buffer = 0;

        channel1Buffer = generateSquareWaveBuffer(soundChannel1, millisecondsSinceLastBuffer);
        channel2Buffer = generateSquareWaveBuffer(soundChannel2, millisecondsSinceLastBuffer);

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

        if (channel1Buffer != 0) {
            delete[] channel1Buffer->buffer;
            delete channel1Buffer;
        }
        if (channel2Buffer != 0) {
            delete[] channel2Buffer->buffer;
            delete channel2Buffer;
        }
        
        sound->pushBuffer(finalBuffer);

        lastStepTime = TIME_MS;

    //}
    // } else {
    //     cout << "Sound off" << endl;
    // }
}

struct AudioBuffer *APU::generateSquareWaveBuffer(SoundChannelSquareWave *squareWaveChannel, 
                                                  long bufferDurationMilliseconds) {

    if (!squareWaveChannel->isChannelEnabled()) return 0;

    // Figure out how many samples to generate
    float samplesPerMillisecond = SAMPLE_FREQUENCY / 1000.0;
    float samplesToGenerate = bufferDurationMilliseconds * samplesPerMillisecond;
    float sampleIntervalMs = bufferDurationMilliseconds / samplesToGenerate;

    struct AudioBuffer *buffer = new struct AudioBuffer;
    buffer->size = samplesToGenerate * BYTES_PER_SAMPLE;
    buffer->buffer = new uint16_t[samplesToGenerate * 2];
    memset(buffer->buffer, 0, buffer->size);
    uint16_t *bufferPointer = buffer->buffer;

    // Length counter is clocked at 256 Hz
    float lengthCounterClocksPerMilliseconds = LENGTH_COUNTER_FREQ / 1000.0;
    float lengthCounterTicksPerSample = lengthCounterClocksPerMilliseconds * sampleIntervalMs;

    // Envelope counter is clocked at 64 Hz
    float envelopeDivisor = squareWaveChannel->getEnvelopeTimerDivisor();
    float envelopeCounterTicksPerMilliseconds = (ENVELOPE_COUNTER_FREQ/envelopeDivisor)/ 1000.0;
    float envelopeCounterTicksPerSample = envelopeCounterTicksPerMilliseconds * sampleIntervalMs;

    // Sweep counter is clocked at 128 Hz, this frequency must be divided by the sweep counter divisor
    float sweepDivisor = squareWaveChannel->getSweepTimerDivisor();
    float sweepCounterTicksPerSample = 0;
    if (sweepDivisor != 0) { 
        float sweepCounterTicksPerMilliseconds = (SWEEP_COUNTER_FREQ/sweepDivisor) / 1000.0;
        sweepCounterTicksPerSample = sweepCounterTicksPerMilliseconds * sampleIntervalMs;
    }

    for (int sample = 0; 
         sample < samplesToGenerate && squareWaveChannel->isChannelEnabled(); 
         sample += 1) {

        generateSquareWaveSample(bufferPointer, squareWaveChannel);

        // Frequency must be recalculated because of sweep.
        float frequencyTimerClocksPerSecond = INPUT_MASTER_CLOCK_HZ / squareWaveChannel->getFrequencyTimerDivisor();
        float frequencyTimerClocksPerMillisecond = frequencyTimerClocksPerSecond / 1000.0;
        float frequencyTimerTicksIncrementPerSample = frequencyTimerClocksPerMillisecond * sampleIntervalMs;
        
        // Update all the timers
        squareWaveChannel->addToLengthTimerTicks(lengthCounterTicksPerSample);
        squareWaveChannel->addToFrequencyTimerTicks(frequencyTimerTicksIncrementPerSample);
        squareWaveChannel->addToEnvelopeTimerTicks(envelopeCounterTicksPerSample);
        squareWaveChannel->addToSweepTimerTicks(sweepCounterTicksPerSample);

        bufferPointer += 2;
    }

    return buffer;
}

void APU::generateSquareWaveSample(uint16_t *bufferPointer, SoundChannelSquareWave *soundChannel) {
    bool outputToLeft = false;
    bool outputToRight = false;
    float volume = 0;

    if (soundChannel->getChannelNumber() == 1) {
        // Output sound 1 to SO1 terminal
        outputToLeft = isBitSet(outputSelection, 0);
        // Output sound 1 to SO2 terminal
        outputToRight = isBitSet(outputSelection, 4);
        volume = getTerminal1Volume();

    } else if (soundChannel->getChannelNumber() == 2) {
        // Output sound 2 to SO1 terminal
        outputToLeft = isBitSet(outputSelection, 1);
        // Output sound 2 to SO2 terminal
        outputToRight = isBitSet(outputSelection, 5);
        volume = getTerminal2Volume();
    }

    volume = (soundChannel->getEnvelopedVolume() + volume) / 2.0;

    uint16_t leftVolume = volume * outputToLeft; // * isTerminal1On();
    uint16_t rightVolume = volume * outputToRight; // * isTerminal2On();

    leftVolume = volumeToOutputVolume(leftVolume);
    rightVolume = volumeToOutputVolume(rightVolume);

    int frequencyTimerTicksModulo8 = (int)soundChannel->getFrequencyTimerTicks() % 8;

    switch (soundChannel->getSoundDuty()) {
        case 0:
            switch (frequencyTimerTicksModulo8) {
                case 0: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 1: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 2: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 3: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 4: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 5: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 6: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 7: *bufferPointer++ =     leftVolume; *bufferPointer++ =     rightVolume; break;
            }
        break;
        case 1:
            switch (frequencyTimerTicksModulo8) {
                case 0: *bufferPointer++ =     leftVolume; *bufferPointer++ =     rightVolume; break;
                case 1: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 2: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 3: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 4: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 5: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 6: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 7: *bufferPointer++ =     leftVolume; *bufferPointer++ =     rightVolume; break;
            }
        break;
        case 2:
            switch (frequencyTimerTicksModulo8) {
                case 0: *bufferPointer++ =     leftVolume; *bufferPointer++ =     rightVolume; break;
                case 1: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 2: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 3: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 4: *bufferPointer++ =              0; *bufferPointer++ =               0; break;
                case 5: *bufferPointer++ =     leftVolume; *bufferPointer++ =     rightVolume; break;
                case 6: *bufferPointer++ =     leftVolume; *bufferPointer++ =     rightVolume; break;
                case 7: *bufferPointer++ =     leftVolume; *bufferPointer++ =     rightVolume; break;
            }
        break;
        case 3:
            switch (frequencyTimerTicksModulo8) {
                case 0: *bufferPointer++ =              0; *bufferPointer++ =                0; break;
                case 1: *bufferPointer++ =     leftVolume; *bufferPointer++ =      rightVolume; break;
                case 2: *bufferPointer++ =     leftVolume; *bufferPointer++ =      rightVolume; break;
                case 3: *bufferPointer++ =     leftVolume; *bufferPointer++ =      rightVolume; break;
                case 4: *bufferPointer++ =     leftVolume; *bufferPointer++ =      rightVolume; break;
                case 5: *bufferPointer++ =     leftVolume; *bufferPointer++ =      rightVolume; break;
                case 6: *bufferPointer++ =     leftVolume; *bufferPointer++ =      rightVolume; break;
                case 7: *bufferPointer++ =              0; *bufferPointer++ =                0; break;
            }
        break;
    }
}

uint16_t APU::volumeToOutputVolume(uint16_t volume) {
    float volumeRatio = (float) volume / 15.0f;
    return 6000 * volumeRatio;
}

bool APU::isTerminal1On() {
    return isBitSet(channelControl, 3);
}

float APU::getTerminal1Volume() {
    float volume = channelControl & 0b111;
    return volume/7.0;
}

bool APU::isTerminal2On() {
    return isBitSet(channelControl, 7);
}

float APU::getTerminal2Volume() {
    float volume = (channelControl & 0b1110000) >> 4;
    return volume/7.0;
}

void APU::write8(uint16_t address, uint8_t value) {
    switch (address) {
        case NR_50_CHANNEL_CONTROL: {
            channelControl = value; 
            break;
        }
        case NR_51_OUTPUT_SELECTION: {
            outputSelection = value;
            break;
        }
        case NR_52_SOUND_ON_OFF: {
            soundOnOff = value; 
            if (isBitSet(soundOnOff, 7)) channelControl |= 0x88;
            break;
        }
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
