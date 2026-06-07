#ifndef CAT_CHESS_SCREEN_SETTINGS_H
#define CAT_CHESS_SCREEN_SETTINGS_H

#include "../../app/app.h"
#include "../../renderer/renderer.h"

void screen_settings_render(Framebuffer* framebuffer, const AppState* app);
void screen_settings_handle_tap(AppState* app, int x, int y);

#endif
