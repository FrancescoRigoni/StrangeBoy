
#ifndef __SOUNDCHANNEL_NOISE_H__
#define __SOUNDCHANNEL_NOISE_H__

#include <cstdint>

#include "Cpu/Memory.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"
#include "Devices/Audio/FrequencyCounter.hpp"
#include "Devices/Audio/LengthCounter.hpp"
#include "Devices/Audio/EnvelopeCounter.hpp"

#define NR_41_SOUND_MODE_LENGTH              0xFF20
#define NR_42_SOUND_MODE_ENVELOPE            0xFF21
#define NR_43_SOUND_MODE_POLY_COUNTER        0xFF22
#define NR_44_SOUND_MODE_FLAGS               0xFF23

using namespace std;

class SoundChannelNoise : public IoDevice {
private:
    uint8_t soundModeLength;
    uint8_t soundModeEnvelope;
    uint8_t soundModePolyCounter;
    uint8_t soundModeFlags;

    bool channelEnabled;

    uint16_t lfsr16;
    uint16_t lfsr8;

    FrequencyCounter frequencyTimer;
    LengthCounter lengthCounter;
    EnvelopeCounter envelopeCounter;

    void checkForTrigger();
    void updatePeriod();

public:
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    bool isChannelEnabled();
    float sample();
};

#endif