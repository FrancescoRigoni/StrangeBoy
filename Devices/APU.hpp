
#ifndef _APU_H_
#define _APU_H_

#include "UI/Sound.hpp"
#include "Devices/SoundChannel1.hpp"
#include "Devices/IoDevice.hpp"

#define NR_50_CHANNEL_CONTROL            0xFF24
#define NR_51_OUTPUT_SELECTION           0xFF25
#define NR_52_SOUND_ON_OFF               0xFF26

class APU : public IoDevice {
private:
    uint8_t channelControl;
    uint8_t outputSelection;
    uint8_t soundOnOff;

    Sound *sound;
    SoundChannel1 *soundChannel1;

    struct AudioBuffer *generateChannel1();
    uint16_t volumeToOutputVolume(uint16_t);

public:
    APU(SoundChannel1 *, Sound *);

    void step();

    virtual void write8(uint16_t, uint8_t);
    virtual uint8_t read8(uint16_t);

};

#endif