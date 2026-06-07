#ifndef CAT_CHESS_GAME_H
#define CAT_CHESS_GAME_H

#include "../chess/chess_board.h"
#include "../input/input.h"

typedef struct {
    ChessBoard board;
    int screenWidth;
    int screenHeight;
    int selectedSquare;
    int tappedSquare;
    int hasSelection;
    int fps;
    int averageFrameMs;
    int exitRequested;
} GameState;

void game_init(GameState* game);
void game_set_screen_size(GameState* game, float width, float height);
void game_update(GameState* game, const InputState* input, float dt);

#endif
