
#ifndef __SOUNDCHANNEL_SQUAREWAVE_H__
#define __SOUNDCHANNEL_SQUAREWAVE_H__

#include <cstdint>
#include <chrono>

#include "Cpu/Memory.hpp"
#include "Util/LogUtil.hpp"
#include "Util/ByteUtil.hpp"
#include "Devices/IoDevice.hpp"

#define NR_10_SOUND_MODE_SWEEP               0xFF10
#define NR_11_SOUND_MODE_LENGTH_DUTY         0xFF11
#define NR_12_SOUND_MODE_ENVELOPE            0xFF12
#define NR_13_SOUND_MODE_FREQ_LO             0xFF13
#define NR_14_SOUND_MODE_FREQ_HI             0xFF14

#define NR_21_SOUND_MODE_LENGTH_DUTY         0xFF16
#define NR_22_SOUND_MODE_ENVELOPE            0xFF17
#define NR_23_SOUND_MODE_FREQ_LO             0xFF18
#define NR_24_SOUND_MODE_FREQ_HI             0xFF19

using namespace std;

class SoundChannelSquareWave : public IoDevice {
private:
    uint16_t regSweepAddress;
    uint16_t regLengthDutyAddress;
    uint16_t regEnvelopeAddress;
    uint16_t regFreqLowAddress;
    uint16_t regFreqHighAddress;

    uint8_t soundModeSweep;
    uint8_t soundModeLengthDuty;
    uint8_t soundModeEnvelope;
    uint8_t soundModeFrequencyLow;
    uint8_t soundModeFrequencyHigh;

    int internalLengthCounter;
    uint16_t frequencyTimerPeriod;
    float frequencyTimerTicks = 0;

    void trigger();

public:
    SoundChannelSquareWave(uint16_t regSweepAddress, 
                           uint16_t regLengthDutyAddress,
                           uint16_t regEnvelopeAddress,
                           uint16_t regFreqLowAddress,
                           uint16_t regFreqHighAddress);

    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

    uint8_t getSoundLength();
    uint8_t getSoundDuty();
    uint16_t getFrequency();
    uint8_t getLength();
    uint8_t getInitialVolume();

    uint8_t getInternalLengthCounter();
    void decrementInternalLengthCounter(int);

    uint16_t getFrequencyTimerPeriod();
    
    float getFrequencyTimerTicks();
    void addToFrequencyTimerTicks(float);

};

#endif