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

static UiRect screen_create_game_create_rect(const AppState* app) {
    UiRect rect;
    rect.width = 360;
    rect.height = 96;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight / 2 + 22;
    return rect;
}

void screen_create_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_CREATE_GAME);
    const char* ready = localization_text(app->settings.locale, LOCALIZED_TEXT_SERVER_API_READY);
    int title_width = font_measure_text(font, 4, title);
    int ready_width = font_measure_text(font, 3, ready);

    ui_draw_label(framebuffer, font, (app->screenWidth - title_width) / 2, app->screenHeight / 2 - 170, 4, 0x27312bffu, title);
    ui_draw_label(framebuffer, font, (app->screenWidth - ready_width) / 2, app->screenHeight / 2 - 42, 3, 0x3d405bffu, ready);
    ui_draw_button_scaled(framebuffer, font, screen_create_game_create_rect(app), title, 3);
    ui_draw_button_scaled(framebuffer, font, screen_create_game_back_rect(app), localization_text(app->settings.locale, LOCALIZED_TEXT_BACK), 3);
}

void screen_create_game_handle_tap(AppState* app, int x, int y) {
    UiRect back_rect;
    UiRect create_rect;
    if (app == 0) {
        return;
    }
    back_rect = screen_create_game_back_rect(app);
    if (ui_rect_contains(&back_rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
        return;
    }

    create_rect = screen_create_game_create_rect(app);
    if (ui_rect_contains(&create_rect, x, y)) {
        app_create_online_game(app);
    }
}
