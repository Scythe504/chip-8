#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>

void audio_init();
void audio_cleanup();
void audio_beep_on();
void audio_beep_off();

#endif
