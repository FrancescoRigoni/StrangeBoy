#include "Devices/Timer.hpp"

/*
INPUT_CLOCK_SELECT:

00: 4.096 KHz (~4.194 KHz SGB) 
01: 262.144 Khz (~268.4 KHz SGB) 
10: 65.536 KHz (~67.11 KHz SGB) 
11: 16.384 KHz
*/

#define INPUT_CLOCK_SELECT (control & 0b11)
#define TIMER_START ((control & 0b100) >> 2)
#define CPU_FREQUENCY_HZ (4*1024*1024)

Timer::Timer(InterruptFlags *interruptFlags) {
    this->interruptFlags = interruptFlags;
}

float Timer::getFrequencyHz() {
    switch(INPUT_CLOCK_SELECT) {
        case 0b00: return 4096;
        case 0b01: return 262144;
        case 0b10: return 65536;
        case 0b11: return 16384;
    }
    return 0;
}

void Timer::tick(int cpuCycles) {
    if (!TIMER_START) {
        return;
    }

    float cpuTimeMilliseconds = ((1.0/CPU_FREQUENCY_HZ)*(float)cpuCycles) * 1000;
    float incrementPerMillisecond = getFrequencyHz() / 1000.0;
    float increments = incrementPerMillisecond * cpuTimeMilliseconds;
    float newCounter = floatValue + increments;

    if (newCounter > 255) {
        floatValue = modulo;
        interruptFlags->interruptTimer();

    } else {
        floatValue = newCounter;
    }
    counter = floatValue;
}

void Timer::write8(uint16_t address, uint8_t value) {
    switch (address) {
        case TIMA: 
            counter = value; floatValue = value; break;

        case TMA: 
            modulo = value; break;

        case TAC: control = value; break;
    }
}

uint8_t Timer::read8(uint16_t address) {
    switch (address) {
        case TIMA: return counter;
        case TMA: return modulo;
        case TAC: return control;
    }
    return 0;
}
