#include "screen_local_game.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../chess_board_view.h"
#include "../ui_controls.h"

static LocalizedTextId screen_local_game_message_text(LocalGameMessage message) {
    if (message == LOCAL_GAME_MESSAGE_AI_THINKING) {
        return LOCALIZED_TEXT_AI_THINKING;
    }
    if (message == LOCAL_GAME_MESSAGE_ILLEGAL_MOVE) {
        return LOCALIZED_TEXT_ILLEGAL_MOVE;
    }
    if (message == LOCAL_GAME_MESSAGE_CHECK) {
        return LOCALIZED_TEXT_CHECK;
    }
    if (message == LOCAL_GAME_MESSAGE_CHECKMATE) {
        return LOCALIZED_TEXT_CHECKMATE;
    }
    if (message == LOCAL_GAME_MESSAGE_STALEMATE) {
        return LOCALIZED_TEXT_STALEMATE;
    }
    if (message == LOCAL_GAME_MESSAGE_YOU_WON) {
        return LOCALIZED_TEXT_YOU_WIN;
    }
    if (message == LOCAL_GAME_MESSAGE_YOU_LOST) {
        return LOCALIZED_TEXT_YOU_LOSE;
    }
    if (message == LOCAL_GAME_MESSAGE_DRAW) {
        return LOCALIZED_TEXT_DRAW;
    }

    return LOCALIZED_TEXT_YOUR_MOVE;
}

static UiRect screen_local_game_back_rect(void) {
    UiRect rect;
    rect.x = 20;
    rect.y = 18;
    rect.width = 150;
    rect.height = 56;
    return rect;
}

void screen_local_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* status = localization_text(
            app->settings.locale,
            screen_local_game_message_text(app->localGameMessage)
    );
    int text_width = font_measure_text(font, 2, status);

    chess_board_view_render(framebuffer, &app->localGame, app->selectedSquare, app->lastTappedSquare);
    ui_draw_button_scaled(
            framebuffer,
            font,
            screen_local_game_back_rect(),
            localization_text(app->settings.locale, LOCALIZED_TEXT_BACK),
            2
    );
    ui_draw_label(
            framebuffer,
            font,
            (app->screenWidth - text_width) / 2,
            88,
            2,
            0x27312bffu,
            status
    );
}

int screen_local_game_handle_tap(AppState* app, int x, int y) {
    UiRect rect = screen_local_game_back_rect();

    if (app == 0) {
        return 0;
    }

    if (!ui_rect_contains(&rect, x, y)) {
        return 0;
    }

    app_navigate(app, APP_SCREEN_HOME);
    return 1;
}
