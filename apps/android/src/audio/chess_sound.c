#include "chess_sound.h"

#include "audio.h"

static const char* chess_sound_id(ChessSoundEvent event) {
    switch (event) {
        case CHESS_SOUND_MOVE:
            return "sfx_move";
        case CHESS_SOUND_CAPTURE:
            return "sfx_capture";
        case CHESS_SOUND_CHECK:
            return "sfx_check";
        case CHESS_SOUND_CHECKMATE:
            return "sfx_checkmate";
        case CHESS_SOUND_ILLEGAL_MOVE:
            return "sfx_illegal";
        case CHESS_SOUND_MENU_SELECT:
            return "sfx_menu_select";
        case CHESS_SOUND_MENU_BACK:
            return "sfx_menu_back";
        case CHESS_SOUND_GAME_CREATED:
            return "sfx_game_created";
        case CHESS_SOUND_GAME_JOINED:
            return "sfx_game_joined";
        case CHESS_SOUND_CASTLE:
            return "sfx_move";
        case CHESS_SOUND_PROMOTION:
            return "sfx_game_created";
        case CHESS_SOUND_GAME_START:
            return "sfx_menu_select";
        case CHESS_SOUND_GAME_END:
            return "sfx_checkmate";
    }

    return 0;
}

void chess_sound_play(ChessSoundEvent event) {
    audio_play_game_sound(chess_sound_id(event));
}
