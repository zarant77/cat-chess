#ifndef CAT_CHESS_ONLINE_GAME_H
#define CAT_CHESS_ONLINE_GAME_H

#include "cat_chess_models.h"

typedef enum {
    ONLINE_GAME_PLACEHOLDER_IDLE = 0,
    ONLINE_GAME_PLACEHOLDER_LOADING,
    ONLINE_GAME_PLACEHOLDER_ERROR
} OnlineGamePlaceholderState;

typedef struct {
    CatChessGameDto game;
    CatChessMoveDto moves[CAT_CHESS_MOVE_LIST_MAX];
    int move_count;
    OnlineGamePlaceholderState state;
} OnlineGame;

void online_game_init(OnlineGame* game);

#endif
