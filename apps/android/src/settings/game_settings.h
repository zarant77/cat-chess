#ifndef CAT_CHESS_SETTINGS_GAME_SETTINGS_H
#define CAT_CHESS_SETTINGS_GAME_SETTINGS_H

#include "../chess/chess_ai.h"

typedef enum {
    GAME_LOCALE_ENGLISH = 0,
    GAME_LOCALE_UKRAINIAN = 1
} GameLocale;

typedef struct {
    int sfx_volume;
    int sounds_enabled;
    int music_enabled;
    int show_move_hints;
    int ai_difficulty;
    int locale;
} GameSettings;

void game_settings_init(GameSettings* settings);
void game_settings_init_with_locale(GameSettings* settings, GameLocale locale);
int game_settings_clamp_volume(int value);
GameLocale game_settings_normalize_locale(int value);
ChessAiDifficulty game_settings_normalize_ai_difficulty(int value);
void game_settings_set_sfx_volume(GameSettings* settings, int value);
void game_settings_set_sounds_enabled(GameSettings* settings, int enabled);
void game_settings_set_music_enabled(GameSettings* settings, int enabled);
void game_settings_set_show_move_hints(GameSettings* settings, int enabled);
void game_settings_set_ai_difficulty(GameSettings* settings, ChessAiDifficulty difficulty);
void game_settings_set_locale(GameSettings* settings, GameLocale locale);
void game_settings_toggle_locale(GameSettings* settings);

#endif
