#include "screen_join_game.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

static int screen_join_game_title_scale(const AppState* app) {
    return app->screenWidth < 420 ? 3 : 4;
}

static int screen_join_game_group_y(const AppState* app) {
    int top;
    int bottom;
    int available;
    int content_height = screen_join_game_title_scale(app) * 20
            + UI_SPACE_XL
            + 104
            + UI_SPACE_MD
            + UI_BUTTON_HEIGHT_PRIMARY
            + UI_SPACE_SM
            + 46;

    top = ui_top_safe_padding(app->screenHeight);
    bottom = app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - UI_BUTTON_HEIGHT_COMPACT - UI_SPACE_MD;
    available = bottom - top;
    if (available < content_height) {
        return top;
    }
    return top + (available - content_height) / 2;
}

static UiRect screen_join_game_invite_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            screen_join_game_group_y(app) + screen_join_game_title_scale(app) * 20 + UI_SPACE_XL,
            width,
            104
    );
}

static UiRect screen_join_game_join_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            screen_join_game_invite_rect(app).y + 104 + UI_SPACE_MD,
            width,
            UI_BUTTON_HEIGHT_PRIMARY
    );
}

static UiRect screen_join_game_error_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            screen_join_game_join_rect(app).y + UI_BUTTON_HEIGHT_PRIMARY + UI_SPACE_SM,
            width,
            46
    );
}

static UiRect screen_join_game_back_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - UI_BUTTON_HEIGHT_COMPACT,
            width,
            UI_BUTTON_HEIGHT_COMPACT
    );
}

static const char* screen_join_game_error_text(const AppState* app) {
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        return 0;
    }
    if (app->lastApiStatus.result == CAT_CHESS_API_NOT_FOUND) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_GAME_NOT_FOUND);
    }
    if (app->lastApiStatus.result == CAT_CHESS_API_CONFLICT) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_INVALID_CODE);
    }
    return localization_text(app->settings.locale, LOCALIZED_TEXT_SERVER_ERROR);
}

void screen_join_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_JOIN_GAME);
    const char* label = localization_text(app->settings.locale, LOCALIZED_TEXT_INVITE_CODE);
    const char* invite = app->inviteCode[0] == '\0' ? "-" : app->inviteCode;
    const char* error = screen_join_game_error_text(app);
    UiRect invite_rect = screen_join_game_invite_rect(app);
    int title_scale = screen_join_game_title_scale(app);

    ui_draw_centered_label(framebuffer, font, app->screenWidth, screen_join_game_group_y(app), title_scale, UI_COLOR_TEXT, title);
    ui_draw_input_field(framebuffer, font, invite_rect, label, invite, app->inviteInputFocused);
    if (error != 0) {
        ui_draw_status_pill(
            framebuffer,
            font,
            screen_join_game_error_rect(app),
            error,
            UI_COLOR_ERROR
        );
    }
    ui_draw_button_style(
            framebuffer,
            font,
            screen_join_game_join_rect(app),
            title,
            app->inviteCode[0] == '\0' ? UI_BUTTON_DISABLED : UI_BUTTON_PRIMARY,
            3
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_join_game_back_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_BACK),
            UI_BUTTON_COMPACT,
            2
    );
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
        app_navigate(app, APP_SCREEN_ONLINE);
        return;
    }

    invite_rect = screen_join_game_invite_rect(app);
    if (ui_rect_contains(&invite_rect, x, y)) {
        app->inviteInputFocused = 1;
        app_request_soft_keyboard(app);
        return;
    }

    join_rect = screen_join_game_join_rect(app);
    if (app->inviteCode[0] != '\0' && ui_rect_contains(&join_rect, x, y)) {
        app_join_online_game(app);
    }
}
