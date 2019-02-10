
#ifndef _APU_H_
#define _APU_H_

#include <chrono>

#include "UI/Sound.hpp"
#include "Devices/SoundChannelSquareWave.hpp"
#include "Devices/IoDevice.hpp"

#define NR_50_CHANNEL_CONTROL            0xFF24
#define NR_51_OUTPUT_SELECTION           0xFF25
#define NR_52_SOUND_ON_OFF               0xFF26

class APU : public IoDevice {
private:
    long lastStepTime;

    uint8_t channelControl;
    uint8_t outputSelection;
    uint8_t soundOnOff;

    Sound *sound;
    SoundChannelSquareWave *soundChannel1;
    SoundChannelSquareWave *soundChannel2;

    struct AudioBuffer *generateSquareWaveBuffer(SoundChannelSquareWave *, long);
    uint16_t volumeToOutputVolume(uint16_t);
    void generateSquareWaveSample(uint16_t *, SoundChannelSquareWave *soundChannel);

    bool isTerminal1On();
    float getTerminal1Volume();

    bool isTerminal2On();
    float getTerminal2Volume();

public:
    APU(SoundChannelSquareWave *, SoundChannelSquareWave *, Sound *);

    void generateOneBuffer();

    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

};

#endif