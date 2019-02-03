
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
    AudioSettings.samples = 800;
    //AudioSettings.callback = &SDLAudioCallback;

    SDL_OpenAudio(&AudioSettings, 0);

}

void Sound::run() {
    struct AudioBuffer *channel1Buffer = popBuffer();
    if (channel1Buffer == 0) return;

    cout << "Playing buffer" << endl;

    if (SDL_GetQueuedAudioSize(1) > 800*60) {
        return;
    }

    SDL_QueueAudio(1, channel1Buffer->buffer, channel1Buffer->size);
    SDL_PauseAudio(0);

    delete[] channel1Buffer->buffer;
    delete channel1Buffer;



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
    // if (queuedBytes > BytesToWrite*2) return;

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