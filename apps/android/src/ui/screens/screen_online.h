#ifndef CAT_CHESS_SCREEN_ONLINE_H
#define CAT_CHESS_SCREEN_ONLINE_H

#include "../../app/app.h"
#include "../../renderer/renderer.h"

void screen_online_render(Framebuffer* framebuffer, const AppState* app);
void screen_online_handle_tap(AppState* app, int x, int y);

#endif
