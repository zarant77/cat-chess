#include "cat_chess_models.h"

void cat_chess_game_dto_init(CatChessGameDto* game) {
    if (game == 0) {
        return;
    }

    game->id = 0;
    game->invite_code[0] = '\0';
    game->status = CAT_CHESS_GAME_STATUS_UNKNOWN;
    game->result = CAT_CHESS_GAME_RESULT_NONE;
    game->board_fen[0] = '\0';
    game->side_to_move = CAT_CHESS_PLAYER_COLOR_NONE;
    game->your_color = CAT_CHESS_PLAYER_COLOR_NONE;
    game->created_at = 0;
    game->updated_at = 0;
    game->started_at = 0;
    game->finished_at = 0;
}

void cat_chess_move_dto_init(CatChessMoveDto* move) {
    if (move == 0) {
        return;
    }

    move->id = 0;
    move->game_id = 0;
    move->move_index = 0;
    move->color = CAT_CHESS_PLAYER_COLOR_NONE;
    move->uci[0] = '\0';
    move->fen_after[0] = '\0';
    move->created_at = 0;
}

void cat_chess_game_list_dto_init(CatChessGameListDto* games) {
    if (games == 0) {
        return;
    }

    games->count = 0;
    for (int index = 0; index < CAT_CHESS_MAX_GAMES; ++index) {
        cat_chess_game_dto_init(games->items + index);
    }
}

void cat_chess_move_list_dto_init(CatChessMoveListDto* moves) {
    if (moves == 0) {
        return;
    }

    moves->count = 0;
    for (int index = 0; index < CAT_CHESS_MAX_MOVES; ++index) {
        cat_chess_move_dto_init(moves->items + index);
    }
}

const char* cat_chess_game_status_string(CatChessGameStatus status) {
    if (status == CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK) {
        return "waiting_for_black";
    }
    if (status == CAT_CHESS_GAME_STATUS_ACTIVE) {
        return "active";
    }
    if (status == CAT_CHESS_GAME_STATUS_FINISHED) {
        return "finished";
    }
    return "unknown";
}

const char* cat_chess_game_result_string(CatChessGameResult result) {
    if (result == CAT_CHESS_GAME_RESULT_NONE) {
        return "none";
    }
    if (result == CAT_CHESS_GAME_RESULT_WHITE_WON) {
        return "white_won";
    }
    if (result == CAT_CHESS_GAME_RESULT_BLACK_WON) {
        return "black_won";
    }
    if (result == CAT_CHESS_GAME_RESULT_DRAW) {
        return "draw";
    }
    return "unknown";
}

const char* cat_chess_player_color_string(CatChessPlayerColor color) {
    if (color == CAT_CHESS_PLAYER_COLOR_WHITE) {
        return "white";
    }
    if (color == CAT_CHESS_PLAYER_COLOR_BLACK) {
        return "black";
    }
    if (color == CAT_CHESS_PLAYER_COLOR_UNKNOWN) {
        return "unknown";
    }
    return "none";
}
