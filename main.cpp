#include <iostream>
#include <fstream>

#include <chrono>
#include <thread>
#include <atomic>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Devices/Io.hpp"
#include "Cpu/Memory.hpp"
#include "Cpu/PersistentRAM.hpp"
#include "PPU/PPU.hpp"
#include "Cpu/Cpu.hpp"

#include "Devices/APU.hpp"
#include "Devices/Dma.hpp"
#include "Devices/LCDRegs.hpp"
#include "Devices/InterruptFlags.hpp"
#include "Devices/DivReg.hpp"
#include "Devices/Timer.hpp"
#include "Devices/Serial.hpp"
#include "Devices/SoundChannelSquareWave.hpp"
#include "Devices/SoundChannelWave.hpp"
#include "Devices/SoundChannelNoise.hpp"

#include "Util/LogUtil.hpp"
#include "Cartridge.hpp"

#include "MBC/MbcDummy.hpp"
#include "MBC/Mbc1.hpp"

#include "UI/UI.hpp"
#include "UI/Screen.hpp"
#include "UI/Sound.hpp"

using namespace std;
using namespace std::chrono;

uint8_t *readRom(const char *);
void runGameBoy(const char *romPath, Screen *, Sound *, Joypad *, atomic<bool> *);

int main(int argc, char **argv) {
    atomic<bool> exit(false);

    UI ui;
    thread gameboyThread(runGameBoy, argv[1], ui.getScreen(), ui.getSound(), ui.getJoypad(), &exit);
    ui.run();

    exit = true;
    gameboyThread.join();
}

unsigned long getTimeMilliseconds() {
    return chrono::duration_cast<chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
}

void runGameBoy(const char *romPath, Screen *screen, Sound *sound, Joypad *joypad, atomic<bool> *exit) {

    uint8_t *bootRom = readRom("roms/bootrom.bin");
    uint8_t *gameRom = readRom(romPath);

    // Read cartridge header
    Cartridge cartridge;
    CartridgeInfo *cartridgeInfo = 0;
    cartridge.parse(gameRom, &cartridgeInfo);

    if (cartridgeInfo->isGBOrSGB != 0x00) {
        cerr << "Unsupported Super GameBoy ROM" << endl;
        //return;
    }

    // Init Memory Bank Controller
    Mbc *memoryBankController;
    switch (cartridgeInfo->cartridgeType) {
        case CART_TYPE_ROM_ONLY:
            // Nothing to do
            memoryBankController = new MbcDummy();
            break;
        case CART_TYPE_ROM_MBC1:
        case CART_TYPE_ROM_MBC1_RAM_BATT:
            memoryBankController = new Mbc1();
            break;
        default:
            cerr << "Unsupported MBC " << cout8Hex(cartridgeInfo->cartridgeType) << endl;
            return;
        break;
    }

    PersistentRAM *persistentRAM = new PersistentRAM(cartridgeInfo->gameTitle);

    Memory memory(bootRom, gameRom, memoryBankController, persistentRAM);
    Dma dma(&memory);
    LCDRegs lcdRegs;
    InterruptFlags interruptFlags;
    DivReg divReg;
    Timer timer(&interruptFlags);
    Serial serial(&interruptFlags);

    SoundChannelSquareWave soundChannel1(NR_10_SOUND_MODE_SWEEP, 
                                         NR_11_SOUND_MODE_LENGTH_DUTY,
                                         NR_12_SOUND_MODE_ENVELOPE, 
                                         NR_13_SOUND_MODE_FREQ_LO, 
                                         NR_14_SOUND_MODE_FREQ_HI);

    SoundChannelSquareWave soundChannel2(0,
                                         NR_21_SOUND_MODE_LENGTH_DUTY,
                                         NR_22_SOUND_MODE_ENVELOPE,
                                         NR_23_SOUND_MODE_FREQ_LO,
                                         NR_24_SOUND_MODE_FREQ_HI);

    SoundChannelWave soundChannel3;
    SoundChannelNoise soundChannel4;

    APU apu(&soundChannel1, &soundChannel2, &soundChannel3, &soundChannel4, sound);
    PPU ppu(&memory, &lcdRegs, &interruptFlags, screen);
    Cpu cpu(&memory, &interruptFlags, &timer, &dma, &divReg);

    memory.registerIoDevice(P1, joypad);
    memory.registerIoDevice(DMA, &dma);
    memory.registerIoDevice(LCDC, &lcdRegs);
    memory.registerIoDevice(STAT, &lcdRegs);
    memory.registerIoDevice(SCY, &lcdRegs);
    memory.registerIoDevice(SCX, &lcdRegs);
    memory.registerIoDevice(LY, &lcdRegs);
    memory.registerIoDevice(LYC, &lcdRegs);
    memory.registerIoDevice(WIN_X, &lcdRegs);
    memory.registerIoDevice(WIN_Y, &lcdRegs);
    memory.registerIoDevice(IF, &interruptFlags);
    memory.registerIoDevice(INTERRUPTS_ENABLE_REG, &interruptFlags);
    memory.registerIoDevice(DIV_REG, &divReg);
    memory.registerIoDevice(TIMA, &timer);
    memory.registerIoDevice(TMA, &timer);
    memory.registerIoDevice(TAC, &timer);
    memory.registerIoDevice(SERIAL_TX_DATA, &serial);
    memory.registerIoDevice(SERIAL_IO_CTRL, &serial);

    memory.registerIoDevice(NR_10_SOUND_MODE_SWEEP, &soundChannel1);
    memory.registerIoDevice(NR_11_SOUND_MODE_LENGTH_DUTY, &soundChannel1);
    memory.registerIoDevice(NR_12_SOUND_MODE_ENVELOPE, &soundChannel1);
    memory.registerIoDevice(NR_13_SOUND_MODE_FREQ_LO, &soundChannel1);
    memory.registerIoDevice(NR_14_SOUND_MODE_FREQ_HI, &soundChannel1);

    memory.registerIoDevice(NR_21_SOUND_MODE_LENGTH_DUTY, &soundChannel2);
    memory.registerIoDevice(NR_22_SOUND_MODE_ENVELOPE, &soundChannel2);
    memory.registerIoDevice(NR_23_SOUND_MODE_FREQ_LO, &soundChannel2);
    memory.registerIoDevice(NR_24_SOUND_MODE_FREQ_HI, &soundChannel2);

    memory.registerIoDevice(NR_30_SOUND_ON_OFF, &soundChannel3);
    memory.registerIoDevice(NR_31_SOUND_LENGTH, &soundChannel3);
    memory.registerIoDevice(NR_32_SOUND_OUTPUT_LEVEL, &soundChannel3);
    memory.registerIoDevice(NR_33_SOUND_MODE_FREQ_LO, &soundChannel3);
    memory.registerIoDevice(NR_34_SOUND_MODE_FREQ_HI, &soundChannel3);
    for (uint16_t waveRamAddress = WAVE_RAM_START; 
         waveRamAddress <= WAVE_RAM_END; 
         waveRamAddress++) {
        memory.registerIoDevice(waveRamAddress, &soundChannel3);
    }

    memory.registerIoDevice(NR_41_SOUND_MODE_LENGTH, &soundChannel4);
    memory.registerIoDevice(NR_42_SOUND_MODE_ENVELOPE, &soundChannel4);
    memory.registerIoDevice(NR_43_SOUND_MODE_POLY_COUNTER, &soundChannel4);
    memory.registerIoDevice(NR_44_SOUND_MODE_FLAGS, &soundChannel4);

    memory.registerIoDevice(NR_50_CHANNEL_CONTROL, &apu);
    memory.registerIoDevice(NR_51_OUTPUT_SELECTION, &apu);
    memory.registerIoDevice(NR_52_SOUND_ON_OFF, &apu);

    int fpsFrequency = 60;
    unsigned long msRefreshPeriod = 1000 / fpsFrequency;

    // Give some time to the screen window to display
    this_thread::sleep_for(chrono::milliseconds(1000));

    do {
        // Draw a frame
        unsigned long timeAtStartOfFrame = getTimeMilliseconds();
        int totalCycles = 0;

        do {
            int cycles = ppu.run();
            totalCycles += cycles;
            cpu.cycle(cycles);
            serial.update();
            ppu.nextState();
        } while (!(lcdRegs.read8(LY) == 0 && lcdRegs.inOAMSearch()));

        apu.generateOneBuffer(totalCycles);

        unsigned long msSpentProcessingFrame = getTimeMilliseconds() - timeAtStartOfFrame;
        int msStillToWaitForNextFrame = msRefreshPeriod - msSpentProcessingFrame;
        int msToSleep = max(0, msStillToWaitForNextFrame);
        this_thread::sleep_for(chrono::milliseconds(msToSleep));

    } while (!cpu.unimplemented && !exit->load());

    return;
}

uint8_t *readRom(const char *fileName) {
    ifstream rom;
    rom.open(fileName, ios::in | ios::binary);

    if (!rom.is_open()) {
        cout << "Unable to open file " << fileName << endl;
        exit(1);
    }

    rom.seekg(0, ios::end);
    int romSize = rom.tellg();
    char *romContent = new char[romSize];
    rom.seekg(0, ios::beg);
    rom.read(romContent, romSize);
    rom.close();

    return (uint8_t*)romContent;
}





