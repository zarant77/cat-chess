#include "screen_my_games.h"

#include <stdio.h>

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

static UiRect screen_my_games_back_rect(const AppState* app) {
    UiRect rect;
    rect.width = 260;
    rect.height = 82;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight / 2 + 142;
    return rect;
}

void screen_my_games_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_MY_GAMES);
    const char* stub_top = localization_text(app->settings.locale, LOCALIZED_TEXT_NETWORK_CLIENT);
    const char* stub_bottom = localization_text(app->settings.locale, LOCALIZED_TEXT_NOT_IMPLEMENTED);
    char columns_top[96];
    char columns_bottom[128];
    int title_width = font_measure_text(font, 4, title);
    int body_top_width;
    int body_bottom_width;
    int stub_top_width = font_measure_text(font, 2, stub_top);
    int stub_bottom_width = font_measure_text(font, 2, stub_bottom);

    snprintf(
            columns_top,
            sizeof(columns_top),
            "%s  %s",
            localization_text(app->settings.locale, LOCALIZED_TEXT_GAME_ID),
            localization_text(app->settings.locale, LOCALIZED_TEXT_STATUS)
    );
    snprintf(
            columns_bottom,
            sizeof(columns_bottom),
            "%s  %s  %s",
            localization_text(app->settings.locale, LOCALIZED_TEXT_YOUR_COLOR),
            localization_text(app->settings.locale, LOCALIZED_TEXT_SIDE_TO_MOVE),
            localization_text(app->settings.locale, LOCALIZED_TEXT_RESULT)
    );
    body_top_width = font_measure_text(font, 2, columns_top);
    body_bottom_width = font_measure_text(font, 2, columns_bottom);

    ui_draw_label(framebuffer, font, (app->screenWidth - title_width) / 2, app->screenHeight / 2 - 170, 4, 0x27312bffu, title);
    ui_draw_label(framebuffer, font, (app->screenWidth - body_top_width) / 2, app->screenHeight / 2 - 54, 2, 0x3d405bffu, columns_top);
    ui_draw_label(framebuffer, font, (app->screenWidth - body_bottom_width) / 2, app->screenHeight / 2 - 14, 2, 0x3d405bffu, columns_bottom);
    ui_draw_label(framebuffer, font, (app->screenWidth - stub_top_width) / 2, app->screenHeight / 2 + 62, 2, 0x3d405bffu, stub_top);
    ui_draw_label(framebuffer, font, (app->screenWidth - stub_bottom_width) / 2, app->screenHeight / 2 + 102, 2, 0x3d405bffu, stub_bottom);
    ui_draw_button_scaled(framebuffer, font, screen_my_games_back_rect(app), localization_text(app->settings.locale, LOCALIZED_TEXT_BACK), 3);
}

void screen_my_games_handle_tap(AppState* app, int x, int y) {
    UiRect rect;
    if (app == 0) {
        return;
    }
    rect = screen_my_games_back_rect(app);
    if (ui_rect_contains(&rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
    }
}
