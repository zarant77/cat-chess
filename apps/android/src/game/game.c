#include "game.h"

#include "../app/app.h"

void game_init(GameState* game) {
    app_init(game);
}

void game_set_screen_size(GameState* game, float width, float height) {
    app_set_screen_size(game, width, height);
}

void game_update(GameState* game, const InputState* input, float dt) {
    app_update(game, input, dt);
}
