#include "chess_move_log_view.h"

#include <stdio.h>

#include "../fonts/font_renderer.h"
#include "../localization/localization.h"
#include "chess_piece_view.h"

#define MOVE_LOG_LINE_MAX 48

static int chess_move_log_view_icon_size(UiRect rect) {
    int size = rect.height < 118 ? 20 : 24;
    if (rect.height < 80) {
        size = 14;
    }
    if (rect.width < 360) {
        size = rect.height < 80 ? 14 : 18;
    }
    return size;
}

static void chess_move_log_view_draw_captured_row(
        Framebuffer* framebuffer,
        UiRect rect,
        const ChessMoveLogEntry* entries,
        int entry_count,
        ChessColor captured_color
) {
    int icon_size = chess_move_log_view_icon_size(rect);
    int gap = 2;
    int x = rect.x;
    int y = rect.y;

    for (int index = 0; index < entry_count; ++index) {
        ChessPiece piece;
        if (entries[index].captured_piece == CHESS_PIECE_NONE
                || entries[index].captured_color != captured_color) {
            continue;
        }

        if (x + icon_size > rect.x + rect.width) {
            x = rect.x;
            y += icon_size + gap;
        }
        if (y + icon_size > rect.y + rect.height) {
            return;
        }

        piece.type = entries[index].captured_piece;
        piece.color = captured_color;
        chess_piece_view_render(framebuffer, piece, x, y, icon_size);
        x += icon_size + gap;
    }
}

static int chess_move_log_view_first_visible_pair(const ChessMoveLogEntry* entries, int entry_count, int max_rows) {
    int last_move_number;
    int first_move_number;

    if (entries == 0 || entry_count <= 0 || max_rows <= 0) {
        return 0;
    }

    last_move_number = entries[entry_count - 1].move_number;
    first_move_number = last_move_number - max_rows + 1;
    if (first_move_number < 1) {
        first_move_number = 1;
    }

    for (int index = 0; index < entry_count; ++index) {
        if (entries[index].move_number >= first_move_number) {
            return index;
        }
    }

    return 0;
}

static void chess_move_log_view_draw_moves(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const ChessMoveLogEntry* entries,
        int entry_count
) {
    int line_height;
    int max_rows;
    int first_index;
    int y;

    if (font == 0 || entries == 0 || entry_count <= 0) {
        return;
    }

    line_height = (int)font->grid_size * 1 + 5;
    max_rows = rect.height / line_height;
    if (max_rows < 1) {
        max_rows = 1;
    }
    first_index = chess_move_log_view_first_visible_pair(entries, entry_count, max_rows);
    y = rect.y;

    for (int index = first_index; index < entry_count && y + line_height <= rect.y + rect.height; ++index) {
        char line[MOVE_LOG_LINE_MAX];
        int text_width;

        if (entries[index].color != CHESS_COLOR_WHITE) {
            continue;
        }

        snprintf(line, sizeof(line), "%d. %s", entries[index].move_number, entries[index].display);
        if (index + 1 < entry_count
                && entries[index + 1].move_number == entries[index].move_number
                && entries[index + 1].color == CHESS_COLOR_BLACK) {
            snprintf(
                    line,
                    sizeof(line),
                    "%d. %s %s",
                    entries[index].move_number,
                    entries[index].display,
                    entries[index + 1].display
            );
        }

        text_width = font_measure_text(font, 1, line);
        while (text_width > rect.width && line[0] != '\0') {
            int length = 0;
            while (line[length] != '\0') {
                ++length;
            }
            if (length <= 1) {
                break;
            }
            line[length - 1] = '\0';
            text_width = font_measure_text(font, 1, line);
        }
        ui_draw_label(framebuffer, font, rect.x, y, 1, UI_COLOR_TEXT, line);
        y += line_height;
    }
}

void chess_move_log_view_render(
        Framebuffer* framebuffer,
        const AppState* app,
        const ChessMoveLogEntry* entries,
        int entry_count,
        UiRect rect
) {
    const PackedFont* font = font_registry_find("vector_16_basic");
    UiRect moves_rect;
    UiRect captured_white_rect;
    UiRect captured_black_rect;
    int label_scale = 1;
    int label_height = 20;
    int gap = 6;
    int captured_width;

    if (framebuffer == 0 || app == 0 || rect.width <= 0 || rect.height <= 0) {
        return;
    }

    renderer_draw_color_rect(framebuffer, rect.x, rect.y, rect.width, rect.height, 0xf9f1e1ccu);
    renderer_draw_color_rect(framebuffer, rect.x, rect.y, rect.width, 2, UI_COLOR_TEXT_MUTED);

    captured_width = rect.width / 3;
    if (captured_width < 112) {
        captured_width = 112;
    }
    if (captured_width > rect.width - 128) {
        captured_width = rect.width / 2;
    }

    moves_rect.x = rect.x + UI_SPACE_SM;
    moves_rect.y = rect.y + label_height;
    moves_rect.width = rect.width - captured_width - UI_SPACE_MD * 2;
    moves_rect.height = rect.height - label_height - gap;

    captured_white_rect.x = rect.x + rect.width - captured_width - UI_SPACE_SM;
    captured_white_rect.y = rect.y + label_height;
    captured_white_rect.width = captured_width;
    captured_white_rect.height = (rect.height - label_height - gap) / 2;

    captured_black_rect = captured_white_rect;
    captured_black_rect.y = captured_white_rect.y + captured_white_rect.height + gap;

    if (font != 0) {
        ui_draw_label(
                framebuffer,
                font,
                moves_rect.x,
                rect.y + 4,
                label_scale,
                UI_COLOR_TEXT_MUTED,
                localization_text(app->settings.locale, LOCALIZED_TEXT_MOVES)
        );
        ui_draw_label(
                framebuffer,
                font,
                captured_white_rect.x,
                rect.y + 4,
                label_scale,
                UI_COLOR_TEXT_MUTED,
                localization_text(app->settings.locale, LOCALIZED_TEXT_CAPTURED)
        );
    }

    chess_move_log_view_draw_moves(framebuffer, font, moves_rect, entries, entry_count);
    chess_move_log_view_draw_captured_row(framebuffer, captured_white_rect, entries, entry_count, CHESS_COLOR_WHITE);
    chess_move_log_view_draw_captured_row(framebuffer, captured_black_rect, entries, entry_count, CHESS_COLOR_BLACK);
}
