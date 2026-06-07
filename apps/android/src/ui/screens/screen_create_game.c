#include "screen_create_game.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

static UiRect screen_create_game_back_rect(const AppState* app) {
    UiRect rect;
    rect.width = 260;
    rect.height = 82;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight / 2 + 142;
    return rect;
}

void screen_create_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_CREATE_GAME);
    const char* ready = localization_text(app->settings.locale, LOCALIZED_TEXT_SERVER_API_READY);
    const char* stub_top = localization_text(app->settings.locale, LOCALIZED_TEXT_NETWORK_CLIENT);
    const char* stub_bottom = localization_text(app->settings.locale, LOCALIZED_TEXT_NOT_IMPLEMENTED);
    int title_width = font_measure_text(font, 4, title);
    int ready_width = font_measure_text(font, 3, ready);
    int stub_top_width = font_measure_text(font, 2, stub_top);
    int stub_bottom_width = font_measure_text(font, 2, stub_bottom);

    ui_draw_label(framebuffer, font, (app->screenWidth - title_width) / 2, app->screenHeight / 2 - 170, 4, 0x27312bffu, title);
    ui_draw_label(framebuffer, font, (app->screenWidth - ready_width) / 2, app->screenHeight / 2 - 42, 3, 0x3d405bffu, ready);
    ui_draw_label(framebuffer, font, (app->screenWidth - stub_top_width) / 2, app->screenHeight / 2 + 28, 2, 0x3d405bffu, stub_top);
    ui_draw_label(framebuffer, font, (app->screenWidth - stub_bottom_width) / 2, app->screenHeight / 2 + 68, 2, 0x3d405bffu, stub_bottom);
    ui_draw_button_scaled(framebuffer, font, screen_create_game_back_rect(app), localization_text(app->settings.locale, LOCALIZED_TEXT_BACK), 3);
}

void screen_create_game_handle_tap(AppState* app, int x, int y) {
    UiRect rect;
    if (app == 0) {
        return;
    }
    rect = screen_create_game_back_rect(app);
    if (ui_rect_contains(&rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
    }
}
