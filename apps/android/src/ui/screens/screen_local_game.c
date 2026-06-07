#include "screen_local_game.h"

#include "../../fonts/font_renderer.h"
#include "../chess_board_view.h"
#include "../ui_controls.h"

void screen_local_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* side = app->localGame.board.sideToMove == CHESS_COLOR_WHITE ? "White to move" : "Black to move";
    int text_width = font_measure_text(font, 2, side);

    chess_board_view_render(framebuffer, &app->localGame, app->selectedSquare, app->lastTappedSquare);
    ui_draw_label(
            framebuffer,
            font,
            (app->screenWidth - text_width) / 2,
            18,
            2,
            0x27312bffu,
            side
    );
}
