#include "audio.h"
#include <stdio.h>
#include <SDL2/SDL.h>

static SDL_AudioDeviceID audio_device = 0;
static int square_wav_pos = 0;

#define SAMPLE_RATE 44100
#define FREQUENCY 440

static void audio_callback(void* userdata, uint8_t* stream, int len) {
  (void)userdata;

  for (unsigned i = 0; i < len; i++) {
    square_wav_pos++;
    int period = SAMPLE_RATE / FREQUENCY;

    stream[i] = (square_wav_pos / (period / 2) % 2 ? 0xF0 : 0x00);
  }
}

void audio_init() {
  SDL_AudioSpec want, have;

  SDL_zero(want);
  want.freq = SAMPLE_RATE;
  want.format = AUDIO_U8;
  want.channels = 1;
  want.samples = 512;
  want.callback = audio_callback;

  audio_device = SDL_OpenAudioDevice(0, 0, &want, &have, 0);

  if (!audio_device) {
    fprintf(stderr, "Audio error: %s\n", SDL_GetError());
    exit(1);
  }

  // start with audio paused
  SDL_PauseAudioDevice(audio_device, 1);
}

void audio_beep_on() {
  if (audio_device) {
    SDL_PauseAudioDevice(audio_device, 0);
  }
}

void audio_beep_off() {
  if (audio_device) {
    SDL_PauseAudioDevice(audio_device, 1);
  }
}

// free audio initializations
void audio_cleanup() {
  if (audio_device) {
    SDL_CloseAudioDevice(audio_device);
    audio_device = 0;
  }
}