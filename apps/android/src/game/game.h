#ifndef CAT_CHESS_GAME_H
#define CAT_CHESS_GAME_H

#include "../app/app_state.h"
#include "../input/input.h"

typedef AppState GameState;

void game_init(GameState* game);
void game_set_screen_size(GameState* game, float width, float height);
void game_update(GameState* game, const InputState* input, float dt);

#endif
