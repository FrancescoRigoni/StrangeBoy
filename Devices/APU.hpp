
#ifndef _APU_H_
#define _APU_H_

#include <chrono>

#include "UI/Sound.hpp"
#include "Devices/SoundChannelSquareWave.hpp"
#include "Devices/SoundChannelWave.hpp"
#include "Devices/SoundChannelNoise.hpp"
#include "Devices/IoDevice.hpp"

#define NR_50_CHANNEL_CONTROL            0xFF24
#define NR_51_OUTPUT_SELECTION           0xFF25
#define NR_52_SOUND_ON_OFF               0xFF26

class APU : public IoDevice {
private:
    long lastBufferTime;

    uint8_t channelControl;
    uint8_t outputSelection;
    uint8_t soundOnOff;

    Sound *sound;
    SoundChannelSquareWave *soundChannel1;
    SoundChannelSquareWave *soundChannel2;
    SoundChannelWave *soundChannel3;
    SoundChannelNoise *soundChannel4;

    uint16_t volumeToOutputVolume(float);

    bool isTerminal1On();
    float getTerminal1Volume();

    bool isTerminal2On();
    float getTerminal2Volume();

    bool soundChannel1ToTerminal1On();
    bool soundChannel1ToTerminal2On();
    bool soundChannel2ToTerminal1On();
    bool soundChannel2ToTerminal2On();
    bool soundChannel3ToTerminal1On();
    bool soundChannel3ToTerminal2On();

public:
    APU(SoundChannelSquareWave *, SoundChannelSquareWave *, SoundChannelWave *, SoundChannelNoise *, Sound *);

    void generateOneBuffer();

    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

};

#endif