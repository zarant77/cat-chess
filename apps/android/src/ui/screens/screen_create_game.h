#ifndef CAT_CHESS_SCREEN_CREATE_GAME_H
#define CAT_CHESS_SCREEN_CREATE_GAME_H

#include "../../app/app.h"
#include "../../renderer/renderer.h"

void screen_create_game_render(Framebuffer* framebuffer, const AppState* app);
void screen_create_game_handle_tap(AppState* app, int x, int y);

#endif
