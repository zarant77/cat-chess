#include "screen_create_game.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

static int screen_create_game_title_scale(const AppState* app) {
    return app->screenWidth < 420 ? 3 : 4;
}

static int screen_create_game_group_y(const AppState* app) {
    int top;
    int bottom;
    int available;
    int content_height = screen_create_game_title_scale(app) * 20
            + UI_SPACE_XL
            + 76
            + UI_SPACE_MD
            + UI_BUTTON_HEIGHT_PRIMARY;

    top = ui_top_safe_padding(app->screenHeight);
    bottom = app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - UI_BUTTON_HEIGHT_COMPACT - UI_SPACE_MD;
    available = bottom - top;
    if (available < content_height) {
        return top;
    }
    return top + (available - content_height) / 2;
}

static UiRect screen_create_game_status_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            screen_create_game_group_y(app) + screen_create_game_title_scale(app) * 20 + UI_SPACE_XL,
            width,
            76
    );
}

static UiRect screen_create_game_create_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            screen_create_game_status_rect(app).y + 76 + UI_SPACE_MD,
            width,
            UI_BUTTON_HEIGHT_PRIMARY
    );
}

static UiRect screen_create_game_back_rect(const AppState* app) {
    int width = ui_content_width(app->screenWidth);
    return ui_centered_rect(
            app->screenWidth,
            app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - UI_BUTTON_HEIGHT_COMPACT,
            width,
            UI_BUTTON_HEIGHT_COMPACT
    );
}

static const char* screen_create_game_server_text(const AppState* app) {
    if (app->onlineGame.is_request_in_flight) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_CONNECTING);
    }
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_SERVER_READY);
    }
    return localization_text(app->settings.locale, LOCALIZED_TEXT_SERVER_ERROR);
}

void screen_create_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_CREATE_GAME);
    int title_scale = screen_create_game_title_scale(app);
    int title_y = screen_create_game_group_y(app);

    ui_draw_centered_label(framebuffer, font, app->screenWidth, title_y, title_scale, UI_COLOR_TEXT, title);
    ui_draw_status_pill(
            framebuffer,
            font,
            screen_create_game_status_rect(app),
            screen_create_game_server_text(app),
            app->lastApiStatus.result == CAT_CHESS_API_OK ? UI_COLOR_TEXT_MUTED : UI_COLOR_ERROR
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_create_game_create_rect(app),
            title,
            app->onlineGame.is_request_in_flight ? UI_BUTTON_DISABLED : UI_BUTTON_PRIMARY,
            3
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_create_game_back_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_BACK),
            UI_BUTTON_COMPACT,
            2
    );
}

void screen_create_game_handle_tap(AppState* app, int x, int y) {
    UiRect back_rect;
    UiRect create_rect;

    if (app == 0) {
        return;
    }

    back_rect = screen_create_game_back_rect(app);
    if (ui_rect_contains(&back_rect, x, y)) {
        app_navigate(app, APP_SCREEN_ONLINE);
        return;
    }

    create_rect = screen_create_game_create_rect(app);
    if (!app->onlineGame.is_request_in_flight && ui_rect_contains(&create_rect, x, y)) {
        app_create_online_game(app);
    }
}
