
#include <cstdint>

#include "Devices/APU.hpp"
#include "Util/ByteUtil.hpp"
#include "Util/LogUtil.hpp"
#include "Devices/Audio/CommonAudio.hpp"

#define NR_52_ALL_ON_OFF_BIT    7
#define NR_52_4_ON_OFF_BIT      3
#define NR_52_3_ON_OFF_BIT      2
#define NR_52_2_ON_OFF_BIT      1
#define NR_52_1_ON_OFF_BIT      0


#define BYTES_PER_SAMPLE        (sizeof(int16_t) * 2)

using namespace std;
using namespace std::chrono;

#define TIME_MS                                                         \
    chrono::duration_cast<chrono::milliseconds>                         \
        (chrono::system_clock::now().time_since_epoch()).count()

APU::APU(SoundChannelSquareWave *soundChannel1, 
         SoundChannelSquareWave *soundChannel2, 
         SoundChannelWave *soundChannel3,
         SoundChannelNoise *soundChannel4, 
         Sound *sound) {

    this->sound = sound;
    this->soundChannel1 = soundChannel1;
    this->soundChannel2 = soundChannel2;
    this->soundChannel3 = soundChannel3;
    this->soundChannel4 = soundChannel4;

    lastBufferTime = TIME_MS;
}

void APU::generateOneBuffer() {
    long millisecondsSinceLastBuffer = (TIME_MS - lastBufferTime);

    // Figure out how many samples to generate
    float sampleIntervalMs = 1.0 / (SAMPLE_FREQUENCY / 1000.0);
    float samplesToGenerate = millisecondsSinceLastBuffer / sampleIntervalMs;
    int samplesInBuffer = samplesToGenerate * 2;

    struct AudioBuffer *audioBuffer;
    audioBuffer = new struct AudioBuffer;
    audioBuffer->size = samplesInBuffer * sizeof(uint16_t);
    audioBuffer->buffer = new uint16_t[samplesInBuffer];
    uint16_t *bufferPointer = audioBuffer->buffer;

    for (int i = 0; i < samplesToGenerate; i++) {
        float terminal1Output = 0;
        float terminal2Output = 0;

        float squareWave1Sample = soundChannel1->isChannelEnabled() ? soundChannel1->sample() : 0;
        float squareWave2Sample = soundChannel2->isChannelEnabled() ? soundChannel2->sample() : 0;
        float waveChannelSample = soundChannel3->isChannelEnabled() ? soundChannel3->sample() : 0;
        float noiseChannelSample = soundChannel4->isChannelEnabled() ? soundChannel4->sample() : 0;

         // cout << "squareWave1Sample: " << squareWave1Sample << endl;
         // cout << "waveChannelSample: " << waveChannelSample << endl;

        if (soundChannel1ToTerminal1On()) terminal1Output += squareWave1Sample;
        if (soundChannel1ToTerminal2On()) terminal2Output += squareWave1Sample;
        if (soundChannel2ToTerminal1On()) terminal1Output += squareWave2Sample;
        if (soundChannel2ToTerminal2On()) terminal2Output += squareWave2Sample;
        if (soundChannel3ToTerminal1On()) terminal1Output += waveChannelSample;
        if (soundChannel3ToTerminal2On()) terminal2Output += waveChannelSample;
        if (soundChannel4ToTerminal1On()) terminal1Output += noiseChannelSample;
        if (soundChannel4ToTerminal2On()) terminal2Output += noiseChannelSample;

        uint16_t leftOutput = volumeToOutputVolume(terminal1Output*getTerminal1Volume());
        uint16_t rightOutput = volumeToOutputVolume(terminal2Output*getTerminal2Volume());

        *bufferPointer++ = leftOutput;
        *bufferPointer++ = rightOutput;
    }

    lastBufferTime = TIME_MS;
    
    sound->pushBuffer(audioBuffer);

}

uint16_t APU::volumeToOutputVolume(float volume) {
    return 3000 * volume;
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

bool APU::soundChannel1ToTerminal1On() {
    return isBitSet(outputSelection, 0);
}

bool APU::soundChannel1ToTerminal2On() {
    return isBitSet(outputSelection, 4);
}

bool APU::soundChannel2ToTerminal1On() {
    return isBitSet(outputSelection, 1);
}

bool APU::soundChannel2ToTerminal2On() {
    return isBitSet(outputSelection, 5);
}

bool APU::soundChannel3ToTerminal1On() {
    return isBitSet(outputSelection, 2);
}

bool APU::soundChannel3ToTerminal2On() {
    return isBitSet(outputSelection, 6);
}

bool APU::soundChannel4ToTerminal1On() {
    return isBitSet(outputSelection, 3);
}

bool APU::soundChannel4ToTerminal2On() {
    return isBitSet(outputSelection, 7);
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
