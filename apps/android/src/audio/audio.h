#ifndef CAT_CHESS_AUDIO_H
#define CAT_CHESS_AUDIO_H

void audio_init(void);
void audio_shutdown(void);
void audio_pause(void);
void audio_resume(void);
void audio_set_sfx_volume(int volume);
void audio_play_sound(const char* id);

#endif
