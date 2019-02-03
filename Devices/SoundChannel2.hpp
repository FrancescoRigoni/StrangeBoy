
#ifndef __SOUNDCHANNEL2_H__
#define __SOUNDCHANNEL2_H__

#include <cstdint>
#include <chrono>
#include "Cpu/Memory.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"

#define NR_21_SOUND_MODE_LENGTH_DUTY         0xFF16
#define NR_22_SOUND_MODE_ENVELOPE            0xFF17
#define NR_23_SOUND_MODE_FREQ_LO             0xFF18
#define NR_24_SOUND_MODE_FREQ_HI             0xFF19

using namespace std;

class SoundChannel2 : public IoDevice {
private:

    uint8_t internalLengthCounter;
    uint16_t internalFrequencyTimerPeriod;

    uint8_t soundModeSweep;
    uint8_t soundModeLengthDuty;
    uint8_t soundModeEnvelope;
    uint8_t soundModeFrequencyLow;
    uint8_t soundModeFrequencyHigh;

    void trigger();

public:
    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    uint8_t getSoundLength();
    uint8_t getSoundDuty();
    uint16_t getFrequency();
    uint8_t getLength();
    uint8_t getInitialVolume();

    uint8_t getInternalLengthCounter();
    uint16_t getInternalFrequencyTimerPeriod();
    void decrementInternalLengthCounter(int);

};

#endif