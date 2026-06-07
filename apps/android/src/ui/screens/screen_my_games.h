#ifndef CAT_CHESS_SCREEN_MY_GAMES_H
#define CAT_CHESS_SCREEN_MY_GAMES_H

#include "../../app/app.h"
#include "../../renderer/renderer.h"

void screen_my_games_render(Framebuffer* framebuffer, const AppState* app);
void screen_my_games_handle_tap(AppState* app, int x, int y);

#endif
