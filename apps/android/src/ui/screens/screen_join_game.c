#include "screen_join_game.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

static UiRect screen_join_game_back_rect(const AppState* app) {
    UiRect rect;
    rect.width = 260;
    rect.height = 82;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight / 2 + 172;
    return rect;
}

static UiRect screen_join_game_join_rect(const AppState* app) {
    UiRect rect;
    rect.width = 260;
    rect.height = 82;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight / 2 + 86;
    return rect;
}

static UiRect screen_join_game_invite_rect(const AppState* app) {
    UiRect rect;
    rect.width = app->screenWidth < 520 ? app->screenWidth - 56 : 520;
    rect.height = 96;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight / 2 - 18;
    return rect;
}

void screen_join_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_JOIN_GAME);
    const char* code = localization_text(app->settings.locale, LOCALIZED_TEXT_INVITE_CODE);
    int title_width = font_measure_text(font, 4, title);
    int code_width = font_measure_text(font, 3, code);
    int invite_width = font_measure_text(font, 3, app->inviteCode[0] == '\0' ? "-" : app->inviteCode);
    UiRect invite_rect = screen_join_game_invite_rect(app);

    ui_draw_label(framebuffer, font, (app->screenWidth - title_width) / 2, app->screenHeight / 2 - 190, 4, 0x27312bffu, title);
    ui_draw_panel(framebuffer, invite_rect);
    ui_draw_label(framebuffer, font, (app->screenWidth - code_width) / 2, invite_rect.y - 44, 3, 0x3d405bffu, code);
    ui_draw_label(framebuffer, font, (app->screenWidth - invite_width) / 2, invite_rect.y + 24, 3, 0x3d405bffu, app->inviteCode[0] == '\0' ? "-" : app->inviteCode);
    ui_draw_button_scaled(framebuffer, font, screen_join_game_join_rect(app), localization_text(app->settings.locale, LOCALIZED_TEXT_JOIN_GAME), 3);
    ui_draw_button_scaled(framebuffer, font, screen_join_game_back_rect(app), localization_text(app->settings.locale, LOCALIZED_TEXT_BACK), 3);
}

void screen_join_game_handle_tap(AppState* app, int x, int y) {
    UiRect back_rect;
    UiRect invite_rect;
    UiRect join_rect;
    if (app == 0) {
        return;
    }
    back_rect = screen_join_game_back_rect(app);
    if (ui_rect_contains(&back_rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
        return;
    }

    invite_rect = screen_join_game_invite_rect(app);
    if (ui_rect_contains(&invite_rect, x, y)) {
        app->inviteInputFocused = 1;
        app_request_soft_keyboard(app);
        return;
    }

    join_rect = screen_join_game_join_rect(app);
    if (ui_rect_contains(&join_rect, x, y)) {
        app_join_online_game(app);
    }
}
