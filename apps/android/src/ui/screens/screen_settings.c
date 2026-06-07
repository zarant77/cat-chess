#include "screen_settings.h"

#include "../../fonts/font_renderer.h"
#include "../ui_controls.h"

static UiRect screen_settings_back_rect(const AppState* app) {
    UiRect rect;
    rect.width = 180;
    rect.height = 52;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight / 2 + 92;
    return rect;
}

void screen_settings_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    int title_width = font_measure_text(font, 3, "Settings");
    int body_width = font_measure_text(font, 2, "Coming soon");

    ui_draw_label(framebuffer, font, (app->screenWidth - title_width) / 2, app->screenHeight / 2 - 80, 3, 0x27312bffu, "Settings");
    ui_draw_label(framebuffer, font, (app->screenWidth - body_width) / 2, app->screenHeight / 2 + 8, 2, 0x3d405bffu, "Coming soon");
    ui_draw_button(framebuffer, font, screen_settings_back_rect(app), "Back");
}

void screen_settings_handle_tap(AppState* app, int x, int y) {
    UiRect rect;
    if (app == 0) {
        return;
    }
    rect = screen_settings_back_rect(app);
    if (ui_rect_contains(&rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
    }
}
