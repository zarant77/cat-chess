#ifndef CAT_CHESS_SCREEN_HOME_H
#define CAT_CHESS_SCREEN_HOME_H

#include "../../app/app.h"
#include "../../renderer/renderer.h"

void screen_home_render(Framebuffer* framebuffer, const AppState* app);
void screen_home_handle_tap(AppState* app, int x, int y);

#endif
