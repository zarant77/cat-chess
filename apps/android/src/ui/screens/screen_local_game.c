#include "screen_local_game.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../chess_board_view.h"
#include "../ui_controls.h"

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
    const char* side = localization_text(
            app->settings.locale,
            app->localGame.board.sideToMove == CHESS_COLOR_WHITE
                    ? LOCALIZED_TEXT_WHITE_TO_MOVE
                    : LOCALIZED_TEXT_BLACK_TO_MOVE
    );
    int text_width = font_measure_text(font, 2, side);

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
            side
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
