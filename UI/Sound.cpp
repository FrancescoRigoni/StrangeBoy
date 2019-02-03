
#include "UI/Sound.hpp"
#include <iostream>

using namespace std;

#define SAMPLE_FREQUENCY        48000
#define BYTES_PER_SAMPLE        (sizeof(int16_t) * 2)
#define SAMPLE_COUNT            800
#define BYTES_PER_BUFFER        (SAMPLE_COUNT * BYTES_PER_SAMPLE)

void Sound::init() {

    SDL_AudioSpec AudioSettings = {0};

    int SamplesPerSecond = 48000;

    AudioSettings.freq = SamplesPerSecond;
    AudioSettings.format = AUDIO_S16;
    AudioSettings.channels = 2;
    AudioSettings.samples = 4096;
    AudioSettings.padding = 0;
    AudioSettings.silence = 0;
    //AudioSettings.callback = &SDLAudioCallback;

    SDL_OpenAudio(&AudioSettings, 0);

}

#define INPUT_MASTER_CLOCK_HZ   4194304

void Sound::run() {
    // int timeSinceLastStep = 8;

    // // A timer generates an output clock every N input clocks, where N is the timer's period.
    // // This is the wave timer's frequency in Hz.
    // int frequencyTimerClocksPerSecond = INPUT_MASTER_CLOCK_HZ / 2000;
    // float frequencyTimerClocksPerMillisecond = frequencyTimerClocksPerSecond / 1000.0;

    // // Figure out how many samples to generate
    // float samplesPerMillisecond = SAMPLE_FREQUENCY / 1000.0;
    // float samplesToGenerate = timeSinceLastStep * samplesPerMillisecond;
    // float sampleIntervalMs = timeSinceLastStep / samplesToGenerate;

    // struct AudioBuffer *buffer = new struct AudioBuffer;
    // buffer->size = samplesToGenerate * BYTES_PER_SAMPLE;
    // buffer->buffer = new uint16_t[samplesToGenerate * 2];
    // uint16_t *bufferPointer = buffer->buffer;

    // int16_t volume = 1000;

    // for (int sample = 0; sample < samplesToGenerate; sample += 1) {
    //     float currentMillisecond = sampleIntervalMs * (float)sample;
    //     float frequencyTimerTicks = frequencyTimerClocksPerMillisecond * currentMillisecond;
    //     // Each waveform takes 8 ticks
    //     int frequencyTimerTicksModulo8 = (int)frequencyTimerTicks % 8;

    //     switch (frequencyTimerTicksModulo8) {
    //         case 0: *bufferPointer++ = volume; *bufferPointer++ = 0; break;
    //         case 1: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
    //         case 2: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
    //         case 3: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
    //         case 4: *bufferPointer++ =      0; *bufferPointer++ =      0; break;
    //         case 5: *bufferPointer++ = volume; *bufferPointer++ = 0; break;
    //         case 6: *bufferPointer++ = volume; *bufferPointer++ = 0; break;
    //         case 7: *bufferPointer++ = volume; *bufferPointer++ = 0; break;
    //     }
    // }


    struct AudioBuffer *buffer = popBuffer();
    if (buffer == 0) return;

    if (SDL_GetQueuedAudioSize(1) == 0) {
        cout << "No audio queued" << endl;
    }

    SDL_QueueAudio(1, buffer->buffer, buffer->size);
    SDL_PauseAudio(0);

    delete[] buffer->buffer;
    delete buffer;



    // int samplesPerSecond = 48000;
    
    // int ToneHz = 256;
    // if (ToneHz == 0) return;

    // //cout << "FREQUENCY: " << ToneHz << endl;

    // int16_t ToneVolume = 3000;
    // uint32_t RunningSampleIndex = 0;
    // int SquareWavePeriod = samplesPerSecond / ToneHz;
    // int HalfSquareWavePeriod = SquareWavePeriod / 2;
    // int BytesPerSample = sizeof(int16_t) * 2;

    // int BytesToWrite = 800 * BytesPerSample;

    // int queuedBytes = SDL_GetQueuedAudioSize(1);
    // cout << "Queued: " << queuedBytes << endl;
    // // if (queuedBytes > BytesToWrite*2) return;

    // void *SoundBuffer = malloc(BytesToWrite);
    // int16_t *SampleOut = (int16_t *)SoundBuffer;
    // int SampleCount = BytesToWrite/BytesPerSample;

    // for(int SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex) {
    //     int16_t SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod) % 2) ? ToneVolume : -ToneVolume;
    //     *SampleOut++ = SampleValue;
    //     *SampleOut++ = SampleValue;

    // }

    // SDL_QueueAudio(1, SoundBuffer, BytesToWrite);
    // // start play audio
    // SDL_PauseAudio(0);
    // free(SoundBuffer);

}

// void Sound::pushFreq(int freq) {
//     unique_lock<mutex> lock(bufferMutex);
//     this->freq = freq;
// }

// int Sound::popFreq() {
//     unique_lock<mutex> lock(bufferMutex);
//     return freq;
// }

void Sound::pushBuffer(struct AudioBuffer *buffer) {
    unique_lock<mutex> lock(bufferMutex);
    channel1BuffersQueue.push(buffer);
}

struct AudioBuffer *Sound::popBuffer() {
    unique_lock<mutex> lock(bufferMutex);

    if (channel1BuffersQueue.size() == 0) {
        return 0;
    }

    struct AudioBuffer *nextBuffer = channel1BuffersQueue.front();
    channel1BuffersQueue.pop();
    return nextBuffer;
}