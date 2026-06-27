#include "chess_game.h"

void chess_game_init(ChessGame* game) {
    if (game == 0) {
        return;
    }

    chess_board_init(&game->board);
}

void chess_game_get_legal_moves_for_square(const ChessGame* game, int square, ChessMoveList* out_moves) {
    ChessMoveList legal_moves;

    if (out_moves == 0) {
        return;
    }

    out_moves->count = 0;
    if (game == 0 || square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    chess_generate_legal_moves(&game->board, &legal_moves);
    for (int index = 0; index < legal_moves.count && out_moves->count < 256; ++index) {
        if (legal_moves.moves[index].from == square) {
            out_moves->moves[out_moves->count] = legal_moves.moves[index];
            out_moves->count += 1;
        }
    }
}
