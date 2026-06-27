#include "screen_my_games.h"

#include <stdio.h>

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../ui_controls.h"

#define MY_GAMES_CARD_HEIGHT 104
#define MY_GAMES_CARD_GAP 14

static UiRect screen_my_games_bottom_rect(const AppState* app, int index) {
    UiRect rect;
    int width = ui_content_width(app->screenWidth);
    int gap = UI_SPACE_SM;

    rect.width = (width - gap) / 2;
    rect.height = UI_BUTTON_HEIGHT_COMPACT;
    rect.x = (app->screenWidth - width) / 2 + index * (rect.width + gap);
    rect.y = app->screenHeight - UI_SPACE_LG - rect.height;
    return rect;
}

static UiRect screen_my_games_card_rect(const AppState* app, int index) {
    int width = ui_content_width(app->screenWidth);
    UiRect rect;
    rect.width = width;
    rect.height = MY_GAMES_CARD_HEIGHT;
    rect.x = (app->screenWidth - width) / 2;
    rect.y = 126 + index * (MY_GAMES_CARD_HEIGHT + MY_GAMES_CARD_GAP);
    return rect;
}

static UiRect screen_my_games_page_rect(const AppState* app, int index) {
    UiRect bottom = screen_my_games_bottom_rect(app, 0);
    UiRect rect = bottom;
    rect.y -= UI_BUTTON_HEIGHT_COMPACT + UI_SPACE_SM;
    rect.x = bottom.x + index * (rect.width + UI_SPACE_SM);
    return rect;
}

static int screen_my_games_visible_count(const AppState* app) {
    int bottom_y = screen_my_games_bottom_rect(app, 0).y - UI_BUTTON_HEIGHT_COMPACT - UI_SPACE_SM;
    int count = 0;

    while (count < app->games.count) {
        UiRect row = screen_my_games_card_rect(app, count);
        if (row.y + row.height > bottom_y - UI_SPACE_MD) {
            break;
        }
        count += 1;
    }

    return count < 1 ? 1 : count;
}

static const char* screen_my_games_status_text(const AppState* app, CatChessGameStatus status) {
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

static const char* screen_my_games_color_text(const AppState* app, CatChessPlayerColor color) {
    if (color == CAT_CHESS_PLAYER_COLOR_WHITE) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_WHITE);
    }
    if (color == CAT_CHESS_PLAYER_COLOR_BLACK) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_BLACK);
    }
    return localization_text(app->settings.locale, LOCALIZED_TEXT_NONE);
}

static const char* screen_my_games_badge_text(const AppState* app, const CatChessGameDto* game) {
    if (game->status == CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_WAITING_FOR_BLACK);
    }
    if (game->status == CAT_CHESS_GAME_STATUS_FINISHED) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_FINISHED);
    }
    if (game->your_color == game->side_to_move) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_YOUR_MOVE);
    }
    return localization_text(app->settings.locale, LOCALIZED_TEXT_OPPONENT_TURN);
}

static int screen_my_games_page_count(const AppState* app) {
    int visible_count = screen_my_games_visible_count(app);
    if (app->games.count <= 0) {
        return 1;
    }
    return (app->games.count + visible_count - 1) / visible_count;
}

void screen_my_games_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    int visible_count = screen_my_games_visible_count(app);
    int page_count = screen_my_games_page_count(app);
    int start_index = app->myGamesPage * visible_count;
    int end_index = start_index + visible_count;

    ui_draw_centered_label(
            framebuffer,
            font,
            app->screenWidth,
            UI_SPACE_XL,
            app->screenWidth < 420 ? 3 : 4,
            UI_COLOR_TEXT,
            localization_text(app->settings.locale, LOCALIZED_TEXT_MY_GAMES)
    );

    if (app->lastApiStatus.result != CAT_CHESS_API_OK) {
        ui_draw_centered_label(
                framebuffer,
                font,
                app->screenWidth,
                170,
                2,
                UI_COLOR_ERROR,
                app->lastApiStatus.error_code[0] != '\0'
                        ? app->lastApiStatus.error_code
                        : localization_text(app->settings.locale, LOCALIZED_TEXT_SERVER_ERROR)
        );
    } else if (app->games.count == 0) {
        ui_draw_centered_label(framebuffer, font, app->screenWidth, 180, 3, UI_COLOR_TEXT, localization_text(app->settings.locale, LOCALIZED_TEXT_NO_GAMES_YET));
        ui_draw_centered_label(framebuffer, font, app->screenWidth, 232, 1, UI_COLOR_TEXT_MUTED, localization_text(app->settings.locale, LOCALIZED_TEXT_CREATE_ONLINE_PROMPT));
    } else {
        if (end_index > app->games.count) {
            end_index = app->games.count;
        }

        for (int index = start_index; index < end_index; ++index) {
            const CatChessGameDto* game = &app->games.items[index];
            UiRect card = screen_my_games_card_rect(app, index - start_index);
            char title[64];
            char subtitle[128];

            snprintf(title, sizeof(title), "%s #%d", localization_text(app->settings.locale, LOCALIZED_TEXT_GAME_ID), game->id);
            snprintf(
                    subtitle,
                    sizeof(subtitle),
                    "%s · %s · %s",
                    screen_my_games_color_text(app, game->your_color),
                    screen_my_games_badge_text(app, game),
                    screen_my_games_status_text(app, game->status)
            );

            ui_draw_card(framebuffer, card);
            ui_draw_label(framebuffer, font, card.x + UI_SPACE_MD, card.y + 18, 2, UI_COLOR_TEXT, title);
            ui_draw_label(framebuffer, font, card.x + UI_SPACE_MD, card.y + 58, 1, UI_COLOR_TEXT_MUTED, subtitle);
        }
    }

    if (page_count > 1 && app->myGamesPage > 0) {
        ui_draw_button_style(framebuffer, font, screen_my_games_page_rect(app, 0), localization_text(app->settings.locale, LOCALIZED_TEXT_PREVIOUS), UI_BUTTON_COMPACT, 1);
    }
    if (page_count > 1 && app->myGamesPage + 1 < page_count) {
        ui_draw_button_style(framebuffer, font, screen_my_games_page_rect(app, 1), localization_text(app->settings.locale, LOCALIZED_TEXT_NEXT), UI_BUTTON_COMPACT, 1);
    }
    ui_draw_button_style(framebuffer, font, screen_my_games_bottom_rect(app, 0), localization_text(app->settings.locale, LOCALIZED_TEXT_BACK), UI_BUTTON_COMPACT, 2);
    ui_draw_button_style(framebuffer, font, screen_my_games_bottom_rect(app, 1), localization_text(app->settings.locale, LOCALIZED_TEXT_RELOAD), UI_BUTTON_SECONDARY, 2);
}

void screen_my_games_handle_tap(AppState* app, int x, int y) {
    UiRect back_rect;
    UiRect reload_rect;
    UiRect previous_rect;
    UiRect next_rect;
    int visible_count;
    int start_index;
    int page_count;

    if (app == 0) {
        return;
    }

    back_rect = screen_my_games_bottom_rect(app, 0);
    if (ui_rect_contains(&back_rect, x, y)) {
        app_navigate(app, APP_SCREEN_HOME);
        return;
    }

    reload_rect = screen_my_games_bottom_rect(app, 1);
    if (ui_rect_contains(&reload_rect, x, y)) {
        app_load_my_games(app);
        return;
    }

    visible_count = screen_my_games_visible_count(app);
    page_count = screen_my_games_page_count(app);
    previous_rect = screen_my_games_page_rect(app, 0);
    if (app->myGamesPage > 0 && ui_rect_contains(&previous_rect, x, y)) {
        app->myGamesPage -= 1;
        return;
    }
    next_rect = screen_my_games_page_rect(app, 1);
    if (app->myGamesPage + 1 < page_count && ui_rect_contains(&next_rect, x, y)) {
        app->myGamesPage += 1;
        return;
    }

    start_index = app->myGamesPage * visible_count;
    for (int index = 0; index < visible_count; ++index) {
        UiRect row = screen_my_games_card_rect(app, index);
        int game_index = start_index + index;
        if (game_index < app->games.count && ui_rect_contains(&row, x, y)) {
            app_open_listed_online_game(app, game_index);
            return;
        }
    }
}
