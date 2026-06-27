#include "screen_local_game.h"

#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../chess_board_view.h"
#include "../chess_move_log_view.h"
#include "../chess_piece_view.h"
#include "../ui_controls.h"

#define LOCAL_GAME_TOP_BUTTON_HEIGHT 82
#define LOCAL_GAME_UNDO_BUTTON_HEIGHT 64
#define LOCAL_GAME_DIALOG_HEIGHT 220
#define LOCAL_GAME_PROMOTION_DIALOG_HEIGHT 156

static int screen_local_game_back_width(const AppState* app) {
    return app->screenWidth < 380 ? 164 : 192;
}

static int screen_local_game_settings_width(const AppState* app) {
    return app->screenWidth < 380 ? 116 : 132;
}

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

static UiRect screen_local_game_back_rect(const AppState* app) {
    UiRect rect;
    rect.x = UI_SPACE_MD;
    rect.y = ui_top_safe_padding(app->screenHeight);
    rect.width = screen_local_game_back_width(app);
    rect.height = LOCAL_GAME_TOP_BUTTON_HEIGHT;
    return rect;
}

static UiRect screen_local_game_status_rect(const AppState* app) {
    UiRect rect;
    rect.x = UI_SPACE_MD + screen_local_game_back_width(app) + UI_SPACE_SM;
    rect.y = ui_top_safe_padding(app->screenHeight);
    rect.width = app->screenWidth - rect.x - screen_local_game_settings_width(app) - UI_SPACE_SM - UI_SPACE_MD;
    rect.height = LOCAL_GAME_TOP_BUTTON_HEIGHT;
    if (rect.width < 128) {
        rect.x = UI_SPACE_MD;
        rect.y += LOCAL_GAME_TOP_BUTTON_HEIGHT + UI_SPACE_XS;
        rect.width = app->screenWidth - UI_SPACE_MD * 2;
    }
    return rect;
}

static UiRect screen_local_game_settings_rect(const AppState* app) {
    UiRect rect;
    rect.width = screen_local_game_settings_width(app);
    rect.height = LOCAL_GAME_TOP_BUTTON_HEIGHT;
    rect.x = app->screenWidth - UI_SPACE_MD - rect.width;
    rect.y = ui_top_safe_padding(app->screenHeight);
    return rect;
}

static UiRect screen_local_game_undo_rect(const AppState* app) {
    UiRect rect;
    rect.width = ui_content_width(app->screenWidth);
    rect.height = LOCAL_GAME_UNDO_BUTTON_HEIGHT;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight - ui_bottom_safe_padding(app->screenHeight) - rect.height;
    return rect;
}

static UiRect screen_local_game_move_log_rect(const AppState* app) {
    ChessBoardViewLayout board = chess_board_view_layout(app->screenWidth, app->screenHeight);
    UiRect undo = screen_local_game_undo_rect(app);
    UiRect rect;
    rect.width = ui_content_width(app->screenWidth);
    rect.height = undo.y - (board.y + board.size) - UI_SPACE_MD;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = board.y + board.size + UI_SPACE_SM;
    if (rect.height > 132) {
        rect.height = 132;
    }
    if (rect.height < 56) {
        rect.height = 56;
        rect.y = undo.y - UI_SPACE_SM - rect.height;
    }
    return rect;
}

static UiRect screen_local_game_confirm_rect(const AppState* app) {
    UiRect rect;
    rect.width = ui_min_int(ui_content_width(app->screenWidth), 520);
    rect.height = LOCAL_GAME_DIALOG_HEIGHT;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = (app->screenHeight - rect.height) / 2;
    return rect;
}

static UiRect screen_local_game_confirm_yes_rect(const AppState* app) {
    UiRect dialog = screen_local_game_confirm_rect(app);
    UiRect rect;
    rect.width = (dialog.width - UI_SPACE_MD * 3) / 2;
    rect.height = UI_BUTTON_HEIGHT_COMPACT;
    rect.x = dialog.x + UI_SPACE_MD;
    rect.y = dialog.y + dialog.height - UI_SPACE_MD - rect.height;
    return rect;
}

static UiRect screen_local_game_confirm_no_rect(const AppState* app) {
    UiRect yes = screen_local_game_confirm_yes_rect(app);
    UiRect rect = yes;
    rect.x = yes.x + yes.width + UI_SPACE_MD;
    return rect;
}

static UiRect screen_local_game_promotion_rect(const AppState* app) {
    UiRect rect;
    rect.width = ui_min_int(ui_content_width(app->screenWidth), 420);
    rect.height = LOCAL_GAME_PROMOTION_DIALOG_HEIGHT;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = (app->screenHeight - rect.height) / 2;
    return rect;
}

static UiRect screen_local_game_promotion_choice_rect(const AppState* app, int index) {
    UiRect dialog = screen_local_game_promotion_rect(app);
    UiRect rect;
    int gap = UI_SPACE_SM;
    rect.width = (dialog.width - UI_SPACE_MD * 2 - gap * 3) / 4;
    rect.height = 82;
    rect.x = dialog.x + UI_SPACE_MD + index * (rect.width + gap);
    rect.y = dialog.y + dialog.height - UI_SPACE_MD - rect.height;
    return rect;
}

static ChessPieceType screen_local_game_promotion_piece(int index) {
    if (index == 1) {
        return CHESS_PIECE_ROOK;
    }
    if (index == 2) {
        return CHESS_PIECE_BISHOP;
    }
    if (index == 3) {
        return CHESS_PIECE_KNIGHT;
    }
    return CHESS_PIECE_QUEEN;
}

static void screen_local_game_draw_promotion_picker(Framebuffer* framebuffer, const AppState* app, const PackedFont* font) {
    UiRect dialog = screen_local_game_promotion_rect(app);
    renderer_draw_color_rect(framebuffer, 0, 0, app->screenWidth, app->screenHeight, 0x00000066u);
    ui_draw_panel(framebuffer, dialog);
    ui_draw_centered_label(framebuffer, font, app->screenWidth, dialog.y + UI_SPACE_MD, 2, UI_COLOR_TEXT_ON_DARK, localization_text(app->settings.locale, LOCALIZED_TEXT_PROMOTION));
    for (int index = 0; index < 4; ++index) {
        UiRect choice = screen_local_game_promotion_choice_rect(app, index);
        ChessPiece piece;
        ui_draw_button_style(framebuffer, font, choice, 0, UI_BUTTON_SECONDARY, 2);
        piece.type = screen_local_game_promotion_piece(index);
        piece.color = app->localGame.board.side_to_move;
        chess_piece_view_render(framebuffer, piece, choice.x + 8, choice.y + 8, choice.height - 16);
    }
}

static void screen_local_game_draw_exit_confirm(Framebuffer* framebuffer, const AppState* app, const PackedFont* font) {
    UiRect dialog = screen_local_game_confirm_rect(app);
    renderer_draw_color_rect(framebuffer, 0, 0, app->screenWidth, app->screenHeight, 0x00000066u);
    ui_draw_panel(framebuffer, dialog);
    ui_draw_centered_label(
            framebuffer,
            font,
            app->screenWidth,
            dialog.y + UI_SPACE_LG,
            2,
            UI_COLOR_TEXT_ON_DARK,
            localization_text(app->settings.locale, LOCALIZED_TEXT_LEAVE_GAME)
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_local_game_confirm_yes_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_YES),
            UI_BUTTON_DANGER,
            2
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_local_game_confirm_no_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_NO),
            UI_BUTTON_SECONDARY,
            2
    );
}

static uint32_t screen_local_game_status_color(LocalGameMessage message) {
    if (message == LOCAL_GAME_MESSAGE_ILLEGAL_MOVE) {
        return UI_COLOR_ERROR;
    }
    if (message == LOCAL_GAME_MESSAGE_CHECK) {
        return UI_COLOR_WARNING;
    }
    if (message == LOCAL_GAME_MESSAGE_CHECKMATE
            || message == LOCAL_GAME_MESSAGE_STALEMATE
            || message == LOCAL_GAME_MESSAGE_YOU_WON
            || message == LOCAL_GAME_MESSAGE_YOU_LOST
            || message == LOCAL_GAME_MESSAGE_DRAW) {
        return UI_COLOR_ACCENT;
    }
    return UI_COLOR_TEXT_MUTED;
}

void screen_local_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const char* status = localization_text(
            app->settings.locale,
            screen_local_game_message_text(app->localGameMessage)
    );
    int hidden_square = app->localAnimation.active ? app->localAnimation.move.to : -1;
    int last_move_from = app->localMoveLogCount > 0 ? app->localMoveLog[app->localMoveLogCount - 1].from : -1;
    int last_move_to = app->localMoveLogCount > 0 ? app->localMoveLog[app->localMoveLogCount - 1].to : -1;

    ui_draw_button_style(
            framebuffer,
            font,
            screen_local_game_back_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_BACK),
            UI_BUTTON_COMPACT,
            3
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_local_game_settings_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_SETTINGS_SHORT),
            UI_BUTTON_COMPACT,
            2
    );
    ui_draw_status_pill(
            framebuffer,
            font,
            screen_local_game_status_rect(app),
            status,
            screen_local_game_status_color(app->localGameMessage)
    );
    chess_board_view_render_for_color_with_feedback(
            framebuffer,
            &app->localGame,
            app->selectedSquare,
            app->lastTappedSquare,
            CHESS_COLOR_WHITE,
            hidden_square,
            app->settings.show_move_hints ? &app->moveHints : 0,
            last_move_from,
            last_move_to
    );
    chess_board_view_render_animated_piece(framebuffer, app->localAnimation, CHESS_COLOR_WHITE);
    chess_move_log_view_render(
            framebuffer,
            app,
            app->localMoveLog,
            app->localMoveLogCount,
            screen_local_game_move_log_rect(app)
    );
    if (app_can_undo_local_game(app)) {
        ui_draw_button_style(
                framebuffer,
                font,
                screen_local_game_undo_rect(app),
                localization_text(app->settings.locale, LOCALIZED_TEXT_UNDO),
                UI_BUTTON_SECONDARY,
                2
        );
    }
    if (app->exitConfirmVisible) {
        screen_local_game_draw_exit_confirm(framebuffer, app, font);
    }
    if (app->promotionPickerVisible && !app->promotionPickerOnline) {
        screen_local_game_draw_promotion_picker(framebuffer, app, font);
    }
}

int screen_local_game_handle_tap(AppState* app, int x, int y) {
    UiRect rect;

    if (app == 0) {
        return 0;
    }

    if (app->promotionPickerVisible && !app->promotionPickerOnline) {
        for (int index = 0; index < 4; ++index) {
            rect = screen_local_game_promotion_choice_rect(app, index);
            if (ui_rect_contains(&rect, x, y)) {
                app_choose_promotion(app, screen_local_game_promotion_piece(index));
                return 1;
            }
        }
        return 1;
    }

    if (app->exitConfirmVisible) {
        rect = screen_local_game_confirm_yes_rect(app);
        if (ui_rect_contains(&rect, x, y)) {
            app_confirm_exit_to_home(app);
            return 1;
        }
        rect = screen_local_game_confirm_no_rect(app);
        if (ui_rect_contains(&rect, x, y)) {
            app_cancel_exit_confirmation(app);
            return 1;
        }
        return 1;
    }

    rect = screen_local_game_undo_rect(app);
    if (app_can_undo_local_game(app) && ui_rect_contains(&rect, x, y)) {
        app_undo_local_game(app);
        return 1;
    }

    rect = screen_local_game_settings_rect(app);
    if (ui_rect_contains(&rect, x, y)) {
        app_open_settings(app);
        return 1;
    }

    rect = screen_local_game_back_rect(app);
    if (!ui_rect_contains(&rect, x, y)) {
        return 0;
    }

    app_request_exit_confirmation(app);
    return 1;
}
