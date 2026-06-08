#include "screen_game.h"

#include <stdio.h>

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../../online/cat_chess_models.h"
#include "../ui_controls.h"

static UiRect screen_game_back_rect(const AppState* app) {
    UiRect rect;
    rect.width = 150;
    rect.height = 48;
    rect.x = 24;
    rect.y = app->screenHeight - rect.height - 24;
    return rect;
}

static UiRect screen_game_resign_rect(const AppState* app) {
    UiRect rect;
    rect.width = 170;
    rect.height = 48;
    rect.x = app->screenWidth - rect.width - 24;
    rect.y = app->screenHeight - rect.height - 24;
    return rect;
}

static void screen_game_draw_row(
        Framebuffer* framebuffer,
        const PackedFont* font,
        int x,
        int y,
        const char* label,
        const char* value
) {
    char row[192];
    snprintf(row, sizeof(row), "%s: %s", label, value == 0 || value[0] == '\0' ? "-" : value);
    ui_draw_label(framebuffer, font, x, y, 1, 0x27312bffu, row);
}

static const char* screen_game_status_text(const AppState* app, CatChessGameStatus status) {
    if (status == CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_WAITING_FOR_BLACK);
    }
    if (status == CAT_CHESS_GAME_STATUS_ACTIVE) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_ACTIVE);
    }
    if (status == CAT_CHESS_GAME_STATUS_FINISHED) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_FINISHED);
    }
    return localization_text(app->settings.locale, LOCALIZED_TEXT_UNKNOWN);
}

static const char* screen_game_result_text(const AppState* app, CatChessGameResult result) {
    if (result == CAT_CHESS_GAME_RESULT_NONE) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_NONE);
    }
    if (result == CAT_CHESS_GAME_RESULT_WHITE_WON) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_WHITE_WON);
    }
    if (result == CAT_CHESS_GAME_RESULT_BLACK_WON) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_BLACK_WON);
    }
    if (result == CAT_CHESS_GAME_RESULT_DRAW) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_DRAW);
    }
    return localization_text(app->settings.locale, LOCALIZED_TEXT_UNKNOWN);
}

static const char* screen_game_color_text(const AppState* app, CatChessPlayerColor color) {
    if (color == CAT_CHESS_PLAYER_COLOR_WHITE) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_WHITE);
    }
    if (color == CAT_CHESS_PLAYER_COLOR_BLACK) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_BLACK);
    }
    return localization_text(app->settings.locale, LOCALIZED_TEXT_NONE);
}

void screen_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const CatChessGameDto* game = &app->onlineGame.game;
    int x = 28;
    int y = 74;
    char id_text[24];
    char moves_text[32];
    const char* title = localization_text(app->settings.locale, LOCALIZED_TEXT_ONLINE_GAME);

    ui_draw_label(framebuffer, font, x, 28, 3, 0x27312bffu, title);

    snprintf(id_text, sizeof(id_text), "%d", game->id);
    screen_game_draw_row(framebuffer, font, x, y, localization_text(app->settings.locale, LOCALIZED_TEXT_GAME_ID), id_text);
    y += 34;
    screen_game_draw_row(framebuffer, font, x, y, localization_text(app->settings.locale, LOCALIZED_TEXT_STATUS), screen_game_status_text(app, game->status));
    y += 34;
    screen_game_draw_row(framebuffer, font, x, y, localization_text(app->settings.locale, LOCALIZED_TEXT_YOUR_COLOR), screen_game_color_text(app, game->your_color));
    y += 34;
    screen_game_draw_row(framebuffer, font, x, y, localization_text(app->settings.locale, LOCALIZED_TEXT_SIDE_TO_MOVE), screen_game_color_text(app, game->side_to_move));
    y += 34;
    screen_game_draw_row(framebuffer, font, x, y, localization_text(app->settings.locale, LOCALIZED_TEXT_RESULT), screen_game_result_text(app, game->result));
    y += 34;
    screen_game_draw_row(framebuffer, font, x, y, localization_text(app->settings.locale, LOCALIZED_TEXT_BOARD_FEN), game->board_fen);
    y += 34;
    snprintf(moves_text, sizeof(moves_text), "%d", app->onlineGame.moves.count);
    screen_game_draw_row(framebuffer, font, x, y, localization_text(app->settings.locale, LOCALIZED_TEXT_MOVES), moves_text);
    y += 52;
    screen_game_draw_row(
            framebuffer,
            font,
            x,
            y,
            localization_text(app->settings.locale, LOCALIZED_TEXT_MOVE_FLOW),
            localization_text(app->settings.locale, LOCALIZED_TEXT_SEND_UCI_ONLY)
    );

    ui_draw_button(framebuffer, font, screen_game_back_rect(app), localization_text(app->settings.locale, LOCALIZED_TEXT_BACK));
    ui_draw_button_colored(
            framebuffer,
            font,
            screen_game_resign_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_RESIGN),
            0x7f3d3dffu,
            0xf2cc8fffu
    );
}

void screen_game_handle_tap(AppState* app, int x, int y) {
    UiRect back_rect;
    UiRect resign_rect;

    if (app == 0) {
        return;
    }

    back_rect = screen_game_back_rect(app);
    if (ui_rect_contains(&back_rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
        return;
    }

    resign_rect = screen_game_resign_rect(app);
    if (ui_rect_contains(&resign_rect, x, y)) {
        app->lastApiStatus = cat_chess_api_resign_game(&app->apiClient, app->onlineGame.game.id, &app->onlineGame.game);
        if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
            app_refresh_online_game(app);
        } else {
            app->networkStatus = NETWORK_STATUS_ERROR;
        }
    }
}
