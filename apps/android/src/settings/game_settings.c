#include "game_settings.h"

int game_settings_clamp_volume(int value)
{
    if (value < 0)
    {
        return 0;
    }

    if (value > 100)
    {
        return 100;
    }

    return value;
}

void game_settings_init(GameSettings* settings)
{
    game_settings_init_with_locale(settings, GAME_LOCALE_ENGLISH);
}

void game_settings_init_with_locale(GameSettings* settings, GameLocale locale)
{
    if (settings == 0)
    {
        return;
    }

    settings->sfx_volume = 80;
    settings->sounds_enabled = 1;
    settings->music_enabled = 1;
    settings->show_move_hints = 1;
    settings->ai_difficulty = CHESS_AI_NORMAL;
    settings->locale = game_settings_normalize_locale(locale);
}

GameLocale game_settings_normalize_locale(int value)
{
    if (value == GAME_LOCALE_UKRAINIAN)
    {
        return GAME_LOCALE_UKRAINIAN;
    }

    return GAME_LOCALE_ENGLISH;
}

ChessAiDifficulty game_settings_normalize_ai_difficulty(int value)
{
    if (value == CHESS_AI_EASY)
    {
        return CHESS_AI_EASY;
    }
    if (value == CHESS_AI_HARD)
    {
        return CHESS_AI_HARD;
    }

    return CHESS_AI_NORMAL;
}

void game_settings_set_sfx_volume(GameSettings* settings, int value)
{
    if (settings == 0)
    {
        return;
    }

    settings->sfx_volume = game_settings_clamp_volume(value);
}

void game_settings_set_sounds_enabled(GameSettings* settings, int enabled)
{
    if (settings == 0)
    {
        return;
    }

    settings->sounds_enabled = enabled ? 1 : 0;
    settings->sfx_volume = settings->sounds_enabled ? game_settings_clamp_volume(settings->sfx_volume) : 0;
    if (settings->sounds_enabled && settings->sfx_volume <= 0)
    {
        settings->sfx_volume = 80;
    }
}

void game_settings_set_music_enabled(GameSettings* settings, int enabled)
{
    if (settings == 0)
    {
        return;
    }

    settings->music_enabled = enabled ? 1 : 0;
}

void game_settings_set_show_move_hints(GameSettings* settings, int enabled)
{
    if (settings == 0)
    {
        return;
    }

    settings->show_move_hints = enabled ? 1 : 0;
}

void game_settings_set_ai_difficulty(GameSettings* settings, ChessAiDifficulty difficulty)
{
    if (settings == 0)
    {
        return;
    }

    settings->ai_difficulty = game_settings_normalize_ai_difficulty(difficulty);
}

void game_settings_set_locale(GameSettings* settings, GameLocale locale)
{
    if (settings == 0)
    {
        return;
    }

    settings->locale = game_settings_normalize_locale(locale);
}

void game_settings_toggle_locale(GameSettings* settings)
{
    if (settings == 0)
    {
        return;
    }

    if (game_settings_normalize_locale(settings->locale) == GAME_LOCALE_UKRAINIAN)
    {
        settings->locale = GAME_LOCALE_ENGLISH;
        return;
    }

    settings->locale = GAME_LOCALE_UKRAINIAN;
}
