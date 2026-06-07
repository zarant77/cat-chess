#include "audio.h"

static int audio_sfx_volume = 80;

static int audio_clamp_volume(int volume) {
    if (volume < 0) {
        return 0;
    }
    if (volume > 100) {
        return 100;
    }
    return volume;
}

void audio_init(void) {
}

void audio_shutdown(void) {
}

void audio_pause(void) {
}

void audio_resume(void) {
}

void audio_set_sfx_volume(int volume) {
    audio_sfx_volume = audio_clamp_volume(volume);
}

void audio_play_sound(const char* id) {
    (void)id;
    (void)audio_sfx_volume;
}
