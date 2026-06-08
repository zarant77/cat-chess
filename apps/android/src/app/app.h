#ifndef CAT_CHESS_APP_H
#define CAT_CHESS_APP_H

#include "app_state.h"
#include "../input/input.h"

void app_init(AppState* app);
void app_set_screen_size(AppState* app, float width, float height);
void app_update(AppState* app, const InputState* input, float dt);
void app_navigate(AppState* app, AppScreen screen);
void app_request_back(AppState* app);
void app_request_soft_keyboard(AppState* app);
int app_take_soft_keyboard_request(AppState* app);
void app_create_online_game(AppState* app);
void app_join_online_game(AppState* app);
void app_refresh_online_game(AppState* app);

#endif
