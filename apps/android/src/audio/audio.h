#ifndef CAT_CHESS_AUDIO_H
#define CAT_CHESS_AUDIO_H

#include <jni.h>

void audio_init(void);
void audio_bind_android(JavaVM* vm, jobject activity);
void audio_shutdown(void);
void audio_pause(void);
void audio_resume(void);
void audio_set_sfx_volume(int volume);
void audio_set_sounds_enabled(int enabled);
int audio_are_sounds_enabled(void);
void audio_set_music_enabled(int enabled);
int audio_is_music_enabled(void);
void audio_play_music(const char* id);
void audio_stop_music(void);
void audio_play_sound(const char* id);
void audio_play_ui_sound(const char* id);
void audio_play_game_sound(const char* id);

#endif
