#include "screen_game.h"

#include <stdio.h>

#include "../../chess/chess_board.h"
#include "../../chess/chess_move_log.h"
#include "../../chess/chess_rules.h"
#include "../../fonts/font_renderer.h"
#include "../../localization/localization.h"
#include "../../online/cat_chess_models.h"
#include "../chess_board_view.h"
#include "../chess_move_log_view.h"
#include "../chess_piece_view.h"
#include "../ui_controls.h"

#define GAME_TOP_BUTTON_HEIGHT 82
#define GAME_DIALOG_HEIGHT 220
#define GAME_PROMOTION_DIALOG_HEIGHT 156

static int screen_game_back_width(const AppState* app) {
    return app->screenWidth < 380 ? 164 : 192;
}

static int screen_game_settings_width(const AppState* app) {
    return app->screenWidth < 380 ? 116 : 132;
}

static UiRect screen_game_back_rect(const AppState* app) {
    UiRect rect;
    rect.width = screen_game_back_width(app);
    rect.height = GAME_TOP_BUTTON_HEIGHT;
    rect.x = UI_SPACE_MD;
    rect.y = ui_top_safe_padding(app->screenHeight);
    return rect;
}

static UiRect screen_game_settings_rect(const AppState* app) {
    UiRect rect;
    rect.width = screen_game_settings_width(app);
    rect.height = GAME_TOP_BUTTON_HEIGHT;
    rect.x = app->screenWidth - UI_SPACE_MD - rect.width;
    rect.y = ui_top_safe_padding(app->screenHeight);
    return rect;
}

static UiRect screen_game_resign_rect(const AppState* app) {
    UiRect rect;
    rect.width = 160;
    rect.height = UI_BUTTON_HEIGHT_COMPACT;
    rect.x = app->screenWidth - rect.width - UI_SPACE_MD;
    rect.y = app->screenHeight - rect.height - UI_SPACE_LG;
    return rect;
}

static UiRect screen_game_status_rect(const AppState* app) {
    UiRect rect;
    rect.x = UI_SPACE_MD + screen_game_back_width(app) + UI_SPACE_SM;
    rect.y = ui_top_safe_padding(app->screenHeight);
    rect.width = app->screenWidth - rect.x - screen_game_settings_width(app) - UI_SPACE_SM - UI_SPACE_MD;
    rect.height = GAME_TOP_BUTTON_HEIGHT;
    if (rect.width < 128) {
        rect.x = UI_SPACE_MD;
        rect.y += GAME_TOP_BUTTON_HEIGHT + UI_SPACE_XS;
        rect.width = app->screenWidth - UI_SPACE_MD * 2;
    }
    return rect;
}

static UiRect screen_game_confirm_rect(const AppState* app) {
    UiRect rect;
    rect.width = ui_min_int(ui_content_width(app->screenWidth), 520);
    rect.height = GAME_DIALOG_HEIGHT;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = (app->screenHeight - rect.height) / 2;
    return rect;
}

static UiRect screen_game_confirm_yes_rect(const AppState* app) {
    UiRect dialog = screen_game_confirm_rect(app);
    UiRect rect;
    rect.width = (dialog.width - UI_SPACE_MD * 3) / 2;
    rect.height = UI_BUTTON_HEIGHT_COMPACT;
    rect.x = dialog.x + UI_SPACE_MD;
    rect.y = dialog.y + dialog.height - UI_SPACE_MD - rect.height;
    return rect;
}

static UiRect screen_game_confirm_no_rect(const AppState* app) {
    UiRect yes = screen_game_confirm_yes_rect(app);
    UiRect rect = yes;
    rect.x = yes.x + yes.width + UI_SPACE_MD;
    return rect;
}

static void screen_game_draw_exit_confirm(Framebuffer* framebuffer, const AppState* app, const PackedFont* font) {
    UiRect dialog = screen_game_confirm_rect(app);
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
            screen_game_confirm_yes_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_YES),
            UI_BUTTON_DANGER,
            2
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_game_confirm_no_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_NO),
            UI_BUTTON_SECONDARY,
            2
    );
}

static UiRect screen_game_promotion_rect(const AppState* app) {
    UiRect rect;
    rect.width = ui_min_int(ui_content_width(app->screenWidth), 420);
    rect.height = GAME_PROMOTION_DIALOG_HEIGHT;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = (app->screenHeight - rect.height) / 2;
    return rect;
}

static UiRect screen_game_promotion_choice_rect(const AppState* app, int index) {
    UiRect dialog = screen_game_promotion_rect(app);
    UiRect rect;
    int gap = UI_SPACE_SM;
    rect.width = (dialog.width - UI_SPACE_MD * 2 - gap * 3) / 4;
    rect.height = 82;
    rect.x = dialog.x + UI_SPACE_MD + index * (rect.width + gap);
    rect.y = dialog.y + dialog.height - UI_SPACE_MD - rect.height;
    return rect;
}

static ChessPieceType screen_game_promotion_piece(int index) {
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

static ChessColor screen_game_chess_color_from_player(CatChessPlayerColor color) {
    return color == CAT_CHESS_PLAYER_COLOR_BLACK ? CHESS_COLOR_BLACK : CHESS_COLOR_WHITE;
}

static void screen_game_draw_promotion_picker(Framebuffer* framebuffer, const AppState* app, const PackedFont* font) {
    UiRect dialog = screen_game_promotion_rect(app);
    ChessColor color = screen_game_chess_color_from_player(app->onlineGame.game.your_color);
    renderer_draw_color_rect(framebuffer, 0, 0, app->screenWidth, app->screenHeight, 0x00000066u);
    ui_draw_panel(framebuffer, dialog);
    ui_draw_centered_label(framebuffer, font, app->screenWidth, dialog.y + UI_SPACE_MD, 2, UI_COLOR_TEXT_ON_DARK, localization_text(app->settings.locale, LOCALIZED_TEXT_PROMOTION));
    for (int index = 0; index < 4; ++index) {
        UiRect choice = screen_game_promotion_choice_rect(app, index);
        ChessPiece piece;
        ui_draw_button_style(framebuffer, font, choice, 0, UI_BUTTON_SECONDARY, 2);
        piece.type = screen_game_promotion_piece(index);
        piece.color = color;
        chess_piece_view_render(framebuffer, piece, choice.x + 8, choice.y + 8, choice.height - 16);
    }
}

static UiRect screen_game_meta_rect(const AppState* app) {
    UiRect rect;
    rect.width = ui_content_width(app->screenWidth);
    rect.height = 38;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight - UI_SPACE_LG - UI_BUTTON_HEIGHT_COMPACT - UI_SPACE_SM - rect.height;
    return rect;
}

static UiRect screen_game_invite_rect(const AppState* app) {
    UiRect rect;
    rect.width = ui_content_width(app->screenWidth);
    rect.height = 88;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = app->screenHeight - UI_SPACE_LG - rect.height;
    return rect;
}

static UiRect screen_game_move_log_rect(const AppState* app) {
    ChessBoardViewLayout board = chess_board_view_layout(app->screenWidth, app->screenHeight);
    UiRect meta = screen_game_meta_rect(app);
    UiRect rect;
    rect.width = ui_content_width(app->screenWidth);
    rect.height = meta.y - (board.y + board.size) - UI_SPACE_MD;
    rect.x = (app->screenWidth - rect.width) / 2;
    rect.y = board.y + board.size + UI_SPACE_SM;
    if (rect.height > 132) {
        rect.height = 132;
    }
    if (rect.height < 56) {
        rect.height = 56;
        rect.y = meta.y - UI_SPACE_SM - rect.height;
    }
    return rect;
}

static int screen_game_build_move_log(const AppState* app, ChessMoveLogEntry* entries, int max_entries) {
    ChessBoard board;
    int count = 0;

    if (app == 0 || entries == 0 || max_entries <= 0) {
        return 0;
    }

    chess_board_set_start_position(&board);
    for (int index = 0; index < app->onlineGame.moves.count && count < max_entries; ++index) {
        ChessMove move;
        ChessBoard before;
        ChessBoard after;

        if (!chess_move_parse_uci(app->onlineGame.moves.items[index].uci, &move)) {
            continue;
        }

        before = board;
        after = board;
        if (!chess_apply_move(&after, &move)) {
            continue;
        }

        chess_move_log_entry_from_move(&before, &move, &after, &entries[count]);
        count += 1;
        board = after;
    }

    return count;
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

static const char* screen_game_turn_text(const AppState* app) {
    const CatChessGameDto* game = &app->onlineGame.game;
    ChessGameStatus board_status;

    if (app->networkStatus == NETWORK_STATUS_ERROR || app->onlineGame.state == ONLINE_GAME_STATE_ERROR) {
        if (app->lastApiStatus.result == CAT_CHESS_API_NOT_FOUND) {
            return localization_text(app->settings.locale, LOCALIZED_TEXT_GAME_NOT_FOUND);
        }
        if (app->lastApiStatus.result == CAT_CHESS_API_NETWORK_ERROR) {
            return localization_text(app->settings.locale, LOCALIZED_TEXT_SERVER_ERROR);
        }
        if (app->lastApiStatus.result == CAT_CHESS_API_NOT_IMPLEMENTED) {
            return localization_text(app->settings.locale, LOCALIZED_TEXT_NETWORK_CLIENT_NOT_IMPLEMENTED);
        }
        return localization_text(app->settings.locale, LOCALIZED_TEXT_SERVER_ERROR);
    }

    if (app->onlineGame.is_request_in_flight
            || app->onlineGame.state == ONLINE_GAME_STATE_CREATING_GAME
            || app->onlineGame.state == ONLINE_GAME_STATE_JOINING_GAME
            || app->onlineGame.state == ONLINE_GAME_STATE_SUBMITTING_MOVE
            || app->onlineGame.state == ONLINE_GAME_STATE_POLLING) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_CONNECTING);
    }

    if (game->status == CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK) {
        if (game->invite_code[0] != '\0') {
            return localization_text(app->settings.locale, LOCALIZED_TEXT_GAME_CREATED);
        }
        return localization_text(app->settings.locale, LOCALIZED_TEXT_WAITING_FOR_OPPONENT);
    }

    board_status = chess_get_game_status(&app->onlineGame.local_game.board);
    if (board_status == CHESS_GAME_CHECKMATE) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_CHECKMATE);
    }
    if (board_status == CHESS_GAME_STALEMATE) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_STALEMATE);
    }
    if (chess_is_in_check(&app->onlineGame.local_game.board, app->onlineGame.local_game.board.side_to_move)) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_CHECK);
    }

    if (game->status == CAT_CHESS_GAME_STATUS_FINISHED) {
        return screen_game_result_text(app, game->result);
    }

    if (game->your_color == game->side_to_move) {
        return localization_text(app->settings.locale, LOCALIZED_TEXT_YOUR_MOVE);
    }

    return localization_text(app->settings.locale, LOCALIZED_TEXT_OPPONENT_TURN);
}

static uint32_t screen_game_status_color(const AppState* app) {
    ChessGameStatus board_status;

    if (app->networkStatus == NETWORK_STATUS_ERROR || app->onlineGame.state == ONLINE_GAME_STATE_ERROR) {
        return UI_COLOR_ERROR;
    }

    board_status = chess_get_game_status(&app->onlineGame.local_game.board);
    if (board_status == CHESS_GAME_CHECKMATE || board_status == CHESS_GAME_STALEMATE) {
        return UI_COLOR_ACCENT;
    }
    if (chess_is_in_check(&app->onlineGame.local_game.board, app->onlineGame.local_game.board.side_to_move)) {
        return UI_COLOR_WARNING;
    }
    if (app->onlineGame.game.status == CAT_CHESS_GAME_STATUS_FINISHED) {
        return UI_COLOR_ACCENT;
    }
    return UI_COLOR_TEXT_MUTED;
}

void screen_game_render(Framebuffer* framebuffer, const AppState* app) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    const CatChessGameDto* game = &app->onlineGame.game;
    ChessColor perspective = game->your_color == CAT_CHESS_PLAYER_COLOR_BLACK ? CHESS_COLOR_BLACK : CHESS_COLOR_WHITE;
    UiRect meta = screen_game_meta_rect(app);
    ChessMoveLogEntry move_log[CHESS_MOVE_LOG_MAX];
    int move_log_count = screen_game_build_move_log(app, move_log, CHESS_MOVE_LOG_MAX);
    char meta_text[96];

    ui_draw_button_style(
            framebuffer,
            font,
            screen_game_back_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_BACK),
            UI_BUTTON_COMPACT,
            3
    );
    ui_draw_button_style(
            framebuffer,
            font,
            screen_game_settings_rect(app),
            localization_text(app->settings.locale, LOCALIZED_TEXT_SETTINGS_SHORT),
            UI_BUTTON_COMPACT,
            2
    );
    ui_draw_status_pill(
            framebuffer,
            font,
            screen_game_status_rect(app),
            screen_game_turn_text(app),
            screen_game_status_color(app)
    );

    chess_board_view_render_for_color_with_feedback(
            framebuffer,
            &app->onlineGame.local_game,
            app->selectedSquare,
            app->lastTappedSquare,
            perspective,
            app->onlineGame.animation.active ? app->onlineGame.animation.move.to : -1,
            app->settings.show_move_hints ? &app->moveHints : 0,
            move_log_count > 0 ? move_log[move_log_count - 1].from : -1,
            move_log_count > 0 ? move_log[move_log_count - 1].to : -1
    );
    chess_board_view_render_animated_piece(framebuffer, app->onlineGame.animation, perspective);
    chess_move_log_view_render(
            framebuffer,
            app,
            move_log,
            move_log_count,
            screen_game_move_log_rect(app)
    );

    if (game->status == CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK && game->invite_code[0] != '\0') {
        ui_draw_input_field(
                framebuffer,
                font,
                screen_game_invite_rect(app),
                localization_text(app->settings.locale, LOCALIZED_TEXT_INVITE_CODE),
                game->invite_code,
                1
        );
        if (app->exitConfirmVisible) {
            screen_game_draw_exit_confirm(framebuffer, app, font);
        }
        if (app->promotionPickerVisible && app->promotionPickerOnline) {
            screen_game_draw_promotion_picker(framebuffer, app, font);
        }
        return;
    }

    if (game->status == CAT_CHESS_GAME_STATUS_ACTIVE) {
        snprintf(
                meta_text,
                sizeof(meta_text),
                "#%d  %s · %s %d",
                game->id,
                screen_game_color_text(app, game->your_color),
                localization_text(app->settings.locale, LOCALIZED_TEXT_MOVES),
                app->onlineGame.moves.count
        );
    } else if (game->invite_code[0] != '\0') {
        snprintf(
                meta_text,
                sizeof(meta_text),
                "#%d  %s %s",
                game->id,
                localization_text(app->settings.locale, LOCALIZED_TEXT_INVITE_CODE),
                game->invite_code
        );
    } else {
        snprintf(
                meta_text,
                sizeof(meta_text),
                "#%d",
                game->id
        );
    }
    ui_draw_status_pill(framebuffer, font, meta, meta_text, UI_COLOR_TEXT_MUTED);

    if (game->status == CAT_CHESS_GAME_STATUS_ACTIVE) {
        ui_draw_button_style(
                framebuffer,
                font,
                screen_game_resign_rect(app),
                localization_text(app->settings.locale, LOCALIZED_TEXT_RESIGN),
                UI_BUTTON_DANGER,
                2
        );
    }
    if (app->exitConfirmVisible) {
        screen_game_draw_exit_confirm(framebuffer, app, font);
    }
    if (app->promotionPickerVisible && app->promotionPickerOnline) {
        screen_game_draw_promotion_picker(framebuffer, app, font);
    }
}

int screen_game_handle_tap(AppState* app, int x, int y) {
    UiRect back_rect;
    UiRect settings_rect;
    UiRect resign_rect;

    if (app == 0) {
        return 0;
    }

    if (app->promotionPickerVisible && app->promotionPickerOnline) {
        for (int index = 0; index < 4; ++index) {
            back_rect = screen_game_promotion_choice_rect(app, index);
            if (ui_rect_contains(&back_rect, x, y)) {
                app_choose_promotion(app, screen_game_promotion_piece(index));
                return 1;
            }
        }
        return 1;
    }

    if (app->exitConfirmVisible) {
        back_rect = screen_game_confirm_yes_rect(app);
        if (ui_rect_contains(&back_rect, x, y)) {
            app_confirm_exit_to_home(app);
            return 1;
        }
        back_rect = screen_game_confirm_no_rect(app);
        if (ui_rect_contains(&back_rect, x, y)) {
            app_cancel_exit_confirmation(app);
            return 1;
        }
        return 1;
    }

    back_rect = screen_game_back_rect(app);
    if (ui_rect_contains(&back_rect, x, y)) {
        app_request_exit_confirmation(app);
        return 1;
    }

    settings_rect = screen_game_settings_rect(app);
    if (ui_rect_contains(&settings_rect, x, y)) {
        app_open_settings(app);
        return 1;
    }

    resign_rect = screen_game_resign_rect(app);
    if (app->onlineGame.game.status == CAT_CHESS_GAME_STATUS_ACTIVE && ui_rect_contains(&resign_rect, x, y)) {
        app->onlineGame.is_request_in_flight = 1;
        app->lastApiStatus = cat_chess_api_resign_game(&app->apiClient, app->onlineGame.game.id, &app->onlineGame.game);
        app->onlineGame.is_request_in_flight = 0;
        if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
            online_game_sync_board_from_game(&app->onlineGame);
            app_refresh_online_game(app);
        } else {
            app->onlineGame.state = ONLINE_GAME_STATE_ERROR;
            app->networkStatus = NETWORK_STATUS_ERROR;
        }
        return 1;
    }

    return 0;
}
