
#ifndef __SOUNDCHANNEL1_H__
#define __SOUNDCHANNEL1_H__

#include <cstdint>
#include <chrono>
#include "Cpu/Memory.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"

#define NR_10_SOUND_MODE_SWEEP               0xFF20
#define NR_11_SOUND_MODE_LENGTH_DUTY         0xFF16
#define NR_12_SOUND_MODE_ENVELOPE            0xFF17
#define NR_13_SOUND_MODE_FREQ_LO             0xFF18
#define NR_14_SOUND_MODE_FREQ_HI             0xFF19

using namespace std;

class SoundChannel1 : public IoDevice {
private:
    uint8_t soundModeSweep;
    uint8_t soundModeLengthDuty;
    uint8_t soundModeEnvelope;
    uint8_t soundModeFrequencyLow;
    uint8_t soundModeFrequencyHigh;

public:
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    uint8_t getSoundLength();
    uint8_t getSoundDuty();
    uint16_t getFrequency();
    uint8_t getLength();
    uint8_t getInitialVolume();

};

#endif