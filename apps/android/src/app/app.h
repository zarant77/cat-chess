#ifndef CAT_CHESS_APP_H
#define CAT_CHESS_APP_H

#include "app_state.h"
#include "../input/input.h"

void app_init(AppState* app);
void app_set_screen_size(AppState* app, float width, float height);
void app_update(AppState* app, const InputState* input, float dt);
void app_navigate(AppState* app, AppScreen screen);
void app_save_settings(AppState* app);
void app_request_back(AppState* app);
void app_open_settings(AppState* app);
void app_request_exit_confirmation(AppState* app);
void app_cancel_exit_confirmation(AppState* app);
void app_confirm_exit_to_home(AppState* app);
void app_choose_promotion(AppState* app, ChessPieceType promotion);
void app_request_soft_keyboard(AppState* app);
int app_take_soft_keyboard_request(AppState* app);
int app_can_undo_local_game(const AppState* app);
void app_undo_local_game(AppState* app);
void app_create_online_game(AppState* app);
void app_join_online_game(AppState* app);
void app_load_my_games(AppState* app);
void app_open_listed_online_game(AppState* app, int index);
void app_refresh_online_game(AppState* app);

#endif
