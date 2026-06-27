#ifndef CAT_CHESS_SCREEN_GAME_H
#define CAT_CHESS_SCREEN_GAME_H

#include "../../app/app.h"
#include "../../renderer/renderer.h"

void screen_game_render(Framebuffer* framebuffer, const AppState* app);
int screen_game_handle_tap(AppState* app, int x, int y);

#endif
