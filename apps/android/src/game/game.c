#include "game.h"

#include "../audio/chess_sound.h"

void game_init(GameState* game) {
    if (game == 0) {
        return;
    }

    chess_board_init(&game->board);
    game->screenWidth = 0;
    game->screenHeight = 0;
    game->selectedSquare = -1;
    game->tappedSquare = -1;
    game->hasSelection = 0;
    game->fps = 0;
    game->averageFrameMs = 0;
    game->exitRequested = 0;
    chess_sound_play(CHESS_SOUND_GAME_START);
}

void game_set_screen_size(GameState* game, float width, float height) {
    if (game == 0) {
        return;
    }

    game->screenWidth = (int)width;
    game->screenHeight = (int)height;
}

static void game_handle_square_tap(GameState* game, int square) {
    ChessPiece piece;
    ChessPiece target;

    if (game == 0 || square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    game->tappedSquare = square;
    piece = chess_board_get_piece(&game->board, square);

    if (!game->hasSelection) {
        if (piece.type != CHESS_PIECE_NONE && piece.color == game->board.sideToMove) {
            game->selectedSquare = square;
            game->hasSelection = 1;
        } else {
            chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        }
        return;
    }

    if (square == game->selectedSquare) {
        game->selectedSquare = -1;
        game->hasSelection = 0;
        return;
    }

    target = chess_board_get_piece(&game->board, square);
    if (!chess_board_move_piece(&game->board, game->selectedSquare, square)) {
        chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        game->selectedSquare = -1;
        game->hasSelection = 0;
        return;
    }

    chess_sound_play(target.type == CHESS_PIECE_NONE ? CHESS_SOUND_MOVE : CHESS_SOUND_CAPTURE);
    game->selectedSquare = -1;
    game->hasSelection = 0;
}

void game_update(GameState* game, const InputState* input, float dt) {
    (void)dt;

    if (game == 0 || input == 0) {
        return;
    }

    if (input->tapReleased) {
        game_handle_square_tap(game, input->tapSquare);
    }
}
