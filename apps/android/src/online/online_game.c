#include "online_game.h"

#include "../chess/chess_fen.h"
#include "../chess/chess_move.h"

void online_game_init(OnlineGame* game) {
    if (game == 0) {
        return;
    }

    cat_chess_game_dto_init(&game->game);
    cat_chess_move_list_dto_init(&game->moves);
    chess_game_init(&game->local_game);
    game->animation.active = 0;
    game->animation.move.from = -1;
    game->animation.move.to = -1;
    game->animation.move.promotion = CHESS_PIECE_NONE;
    game->animation.piece = chess_piece_empty();
    game->animation.elapsed_ms = 0.0f;
    game->animation.duration_ms = CHESS_MOVE_ANIMATION_MS;
    game->state = ONLINE_GAME_STATE_OFFLINE;
    game->synced_updated_at = 0;
    game->poll_elapsed_ms = 0.0f;
    game->is_request_in_flight = 0;
}

int online_game_sync_board_from_game(OnlineGame* game) {
    if (game == 0 || game->game.board_fen[0] == '\0') {
        return 0;
    }

    if (!chess_board_from_fen(&game->local_game.board, game->game.board_fen)) {
        return 0;
    }

    game->synced_updated_at = game->game.updated_at;
    if (game->game.status == CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK) {
        game->state = ONLINE_GAME_STATE_WAITING_FOR_OPPONENT;
    } else if (game->game.status == CAT_CHESS_GAME_STATUS_ACTIVE) {
        game->state = ONLINE_GAME_STATE_ACTIVE;
    } else if (game->game.status == CAT_CHESS_GAME_STATUS_FINISHED) {
        game->state = ONLINE_GAME_STATE_FINISHED;
    }
    return 1;
}

int online_game_is_waiting_or_active(const OnlineGame* game) {
    if (game == 0 || game->game.id <= 0) {
        return 0;
    }

    return game->game.status == CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK
            || game->game.status == CAT_CHESS_GAME_STATUS_ACTIVE;
}

int online_game_is_players_turn(const OnlineGame* game) {
    if (game == 0 || game->game.status != CAT_CHESS_GAME_STATUS_ACTIVE) {
        return 0;
    }

    return game->game.your_color != CAT_CHESS_PLAYER_COLOR_NONE
            && game->game.your_color == game->game.side_to_move;
}

void online_game_start_move_animation(OnlineGame* game, const char* uci, ChessPiece piece) {
    ChessMove move;

    if (game == 0 || uci == 0 || !chess_move_parse_uci(uci, &move) || piece.type == CHESS_PIECE_NONE) {
        return;
    }

    game->animation.active = 1;
    game->animation.move = move;
    game->animation.piece = piece;
    game->animation.elapsed_ms = 0.0f;
    game->animation.duration_ms = CHESS_MOVE_ANIMATION_MS;
}

void online_game_update_animation(OnlineGame* game, float dt) {
    if (game == 0 || !game->animation.active) {
        return;
    }

    game->animation.elapsed_ms += dt * 1000.0f;
    if (game->animation.elapsed_ms >= game->animation.duration_ms) {
        online_game_finish_animation(game);
    }
}

void online_game_finish_animation(OnlineGame* game) {
    if (game == 0) {
        return;
    }

    game->animation.active = 0;
    game->animation.elapsed_ms = 0.0f;
}
