#include "chess_board_view.h"

#include "../ui/chess_piece_view.h"
#include "ui_controls.h"

static int min_int(int left, int right) {
    return left < right ? left : right;
}

ChessBoardViewLayout chess_board_view_layout(int screen_width, int screen_height) {
    ChessBoardViewLayout layout;
    int margin = UI_BOARD_MARGIN;
    int top_reserved = ui_top_safe_padding(screen_height) + 92;
    int bottom_reserved = screen_height < 640 ? 172 : 188;
    int available_width = screen_width - margin * 2;
    int available_height;

    if (screen_width < 470) {
        top_reserved += 70;
    }

    available_height = screen_height - top_reserved - bottom_reserved;

    if (available_width < 8) {
        available_width = 8;
    }
    if (available_height < 8) {
        available_height = 8;
    }

    layout.squareSize = min_int(available_width, available_height) / CHESS_BOARD_SIZE;
    if (layout.squareSize < 1) {
        layout.squareSize = 1;
    }
    layout.size = layout.squareSize * CHESS_BOARD_SIZE;
    layout.x = (screen_width - layout.size) / 2;
    layout.y = top_reserved + (available_height - layout.size) / 2;

    return layout;
}

int chess_board_view_square_at(int screen_width, int screen_height, float x, float y) {
    return chess_board_view_square_at_for_color(screen_width, screen_height, x, y, CHESS_COLOR_WHITE);
}

int chess_board_view_square_at_for_color(
        int screen_width,
        int screen_height,
        float x,
        float y,
        ChessColor perspective
) {
    ChessBoardViewLayout layout = chess_board_view_layout(screen_width, screen_height);
    int file;
    int rank;

    if (x < (float)layout.x
            || y < (float)layout.y
            || x >= (float)(layout.x + layout.size)
            || y >= (float)(layout.y + layout.size)) {
        return -1;
    }

    file = ((int)x - layout.x) / layout.squareSize;
    rank = CHESS_BOARD_SIZE - 1 - (((int)y - layout.y) / layout.squareSize);
    if (perspective == CHESS_COLOR_BLACK) {
        file = CHESS_BOARD_SIZE - 1 - file;
        rank = CHESS_BOARD_SIZE - 1 - rank;
    }

    return chess_board_square(file, rank);
}

static void chess_board_view_draw_border(Framebuffer* framebuffer, ChessBoardViewLayout layout) {
    renderer_draw_color_rect(framebuffer, layout.x - 6, layout.y - 6, layout.size + 12, 6, 0x263244ffu);
    renderer_draw_color_rect(framebuffer, layout.x - 6, layout.y + layout.size, layout.size + 12, 6, 0x263244ffu);
    renderer_draw_color_rect(framebuffer, layout.x - 6, layout.y, 6, layout.size, 0x263244ffu);
    renderer_draw_color_rect(framebuffer, layout.x + layout.size, layout.y, 6, layout.size, 0x263244ffu);
}

void chess_board_view_render(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square
) {
    chess_board_view_render_for_color(framebuffer, game, selected_square, last_tapped_square, CHESS_COLOR_WHITE);
}

void chess_board_view_render_for_color(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square,
        ChessColor perspective
) {
    chess_board_view_render_for_color_with_hidden_square(
            framebuffer,
            game,
            selected_square,
            last_tapped_square,
            perspective,
            -1
    );
}

void chess_board_view_render_for_color_with_hidden_square(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square,
        ChessColor perspective,
        int hidden_square
) {
    chess_board_view_render_for_color_with_hidden_square_and_hints(
            framebuffer,
            game,
            selected_square,
            last_tapped_square,
            perspective,
            hidden_square,
            0
    );
}

static int chess_board_view_has_hint(const ChessMoveList* hints, int square) {
    if (hints == 0) {
        return 0;
    }

    for (int index = 0; index < hints->count; ++index) {
        if (hints->moves[index].to == square) {
            return 1;
        }
    }

    return 0;
}

static void chess_board_view_draw_move_hint(
        Framebuffer* framebuffer,
        int x,
        int y,
        int size,
        int capture
) {
    int dot_size = ui_max_int(8, size / 5);
    int dot_x = x + (size - dot_size) / 2;
    int dot_y = y + (size - dot_size) / 2;
    int inset = ui_max_int(6, size / 8);
    int thickness = ui_max_int(3, size / 18);
    uint32_t quiet_color = 0x26324488u;
    uint32_t capture_color = 0xf4c542ccu;

    if (!capture) {
        renderer_draw_color_rect(framebuffer, dot_x, dot_y, dot_size, dot_size, quiet_color);
        return;
    }

    renderer_draw_color_rect(framebuffer, x + inset, y + inset, size - inset * 2, thickness, capture_color);
    renderer_draw_color_rect(framebuffer, x + inset, y + size - inset - thickness, size - inset * 2, thickness, capture_color);
    renderer_draw_color_rect(framebuffer, x + inset, y + inset, thickness, size - inset * 2, capture_color);
    renderer_draw_color_rect(framebuffer, x + size - inset - thickness, y + inset, thickness, size - inset * 2, capture_color);
}

static int chess_board_view_king_square(const ChessBoard* board, ChessColor color) {
    if (board == 0) {
        return -1;
    }

    for (int square = 0; square < CHESS_BOARD_SQUARE_COUNT; ++square) {
        ChessPiece piece = chess_board_get_piece(board, square);
        if (piece.type == CHESS_PIECE_KING && piece.color == color) {
            return square;
        }
    }

    return -1;
}

static void chess_board_view_draw_check_warning(Framebuffer* framebuffer, int x, int y, int size) {
    int thickness = ui_max_int(4, size / 12);
    int inset = ui_max_int(3, size / 18);
    renderer_draw_color_rect(framebuffer, x + inset, y + inset, size - inset * 2, size - inset * 2, 0xd12f2f44u);
    renderer_draw_color_rect(framebuffer, x + inset, y + inset, size - inset * 2, thickness, 0xd12f2fffu);
    renderer_draw_color_rect(framebuffer, x + inset, y + size - inset - thickness, size - inset * 2, thickness, 0xd12f2fffu);
    renderer_draw_color_rect(framebuffer, x + inset, y + inset, thickness, size - inset * 2, 0xd12f2fffu);
    renderer_draw_color_rect(framebuffer, x + size - inset - thickness, y + inset, thickness, size - inset * 2, 0xd12f2fffu);
}

static void chess_board_view_draw_last_move(Framebuffer* framebuffer, int x, int y, int size) {
    int thickness = ui_max_int(3, size / 18);
    renderer_draw_color_rect(framebuffer, x, y, size, size, 0xf4c54255u);
    renderer_draw_color_rect(framebuffer, x, y, size, thickness, 0xf4c542aau);
    renderer_draw_color_rect(framebuffer, x, y + size - thickness, size, thickness, 0xf4c542aau);
    renderer_draw_color_rect(framebuffer, x, y, thickness, size, 0xf4c542aau);
    renderer_draw_color_rect(framebuffer, x + size - thickness, y, thickness, size, 0xf4c542aau);
}

void chess_board_view_render_for_color_with_hidden_square_and_hints(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square,
        ChessColor perspective,
        int hidden_square,
        const ChessMoveList* move_hints
) {
    chess_board_view_render_for_color_with_feedback(
            framebuffer,
            game,
            selected_square,
            last_tapped_square,
            perspective,
            hidden_square,
            move_hints,
            -1,
            -1
    );
}

void chess_board_view_render_for_color_with_feedback(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square,
        ChessColor perspective,
        int hidden_square,
        const ChessMoveList* move_hints,
        int last_move_from,
        int last_move_to
) {
    ChessBoardViewLayout layout;
    int checked_king_square = -1;

    if (framebuffer == 0 || game == 0) {
        return;
    }

    layout = chess_board_view_layout(framebuffer->width, framebuffer->height);
    if (chess_is_in_check(&game->board, game->board.side_to_move)) {
        checked_king_square = chess_board_view_king_square(&game->board, game->board.side_to_move);
    }
    chess_board_view_draw_border(framebuffer, layout);

    for (int view_rank = 0; view_rank < CHESS_BOARD_SIZE; ++view_rank) {
        for (int view_file = 0; view_file < CHESS_BOARD_SIZE; ++view_file) {
            int file = view_file;
            int rank = CHESS_BOARD_SIZE - 1 - view_rank;
            int square = chess_board_square(file, rank);
            int x = layout.x + view_file * layout.squareSize;
            int y = layout.y + view_rank * layout.squareSize;
            uint32_t color = ((file + rank) & 1) ? 0x78936cffu : 0xf1dfbaffu;

            if (perspective == CHESS_COLOR_BLACK) {
                file = CHESS_BOARD_SIZE - 1 - view_file;
                rank = view_rank;
                square = chess_board_square(file, rank);
                color = ((file + rank) & 1) ? 0x78936cffu : 0xf1dfbaffu;
            }

            renderer_draw_color_rect(framebuffer, x, y, layout.squareSize, layout.squareSize, color);

            if (square == last_move_from || square == last_move_to) {
                chess_board_view_draw_last_move(framebuffer, x, y, layout.squareSize);
            }
            if (square == checked_king_square) {
                chess_board_view_draw_check_warning(framebuffer, x, y, layout.squareSize);
            }

            if (selected_square >= 0 && square == selected_square) {
                renderer_draw_color_rect(framebuffer, x, y, layout.squareSize, 4, 0xf4c542ffu);
                renderer_draw_color_rect(framebuffer, x, y + layout.squareSize - 4, layout.squareSize, 4, 0xf4c542ffu);
                renderer_draw_color_rect(framebuffer, x, y, 4, layout.squareSize, 0xf4c542ffu);
                renderer_draw_color_rect(framebuffer, x + layout.squareSize - 4, y, 4, layout.squareSize, 0xf4c542ffu);
            } else if (square == last_tapped_square) {
                renderer_draw_color_rect(framebuffer, x + 4, y + 4, layout.squareSize - 8, layout.squareSize - 8, 0xffffff33u);
            }

            if (chess_board_view_has_hint(move_hints, square)) {
                ChessPiece target = chess_board_get_piece(&game->board, square);
                chess_board_view_draw_move_hint(
                        framebuffer,
                        x,
                        y,
                        layout.squareSize,
                        target.type != CHESS_PIECE_NONE
                );
            }

            chess_piece_view_render(
                    framebuffer,
                    square == hidden_square ? chess_piece_empty() : chess_board_get_piece(&game->board, square),
                    x,
                    y,
                    layout.squareSize
            );
        }
    }
}

static void chess_board_view_square_center(
        ChessBoardViewLayout layout,
        int square,
        ChessColor perspective,
        float* out_x,
        float* out_y
) {
    int file = chess_board_file(square);
    int rank = chess_board_rank(square);
    int view_file = file;
    int view_rank = CHESS_BOARD_SIZE - 1 - rank;

    if (perspective == CHESS_COLOR_BLACK) {
        view_file = CHESS_BOARD_SIZE - 1 - file;
        view_rank = rank;
    }

    if (out_x != 0) {
        *out_x = (float)(layout.x + view_file * layout.squareSize);
    }
    if (out_y != 0) {
        *out_y = (float)(layout.y + view_rank * layout.squareSize);
    }
}

void chess_board_view_render_animated_piece(
        Framebuffer* framebuffer,
        ChessMoveAnimation animation,
        ChessColor perspective
) {
    ChessBoardViewLayout layout;
    float from_x;
    float from_y;
    float to_x;
    float to_y;
    float t;
    float visual_t;
    int x;
    int y;

    if (framebuffer == 0 || !animation.active || animation.piece.type == CHESS_PIECE_NONE) {
        return;
    }

    layout = chess_board_view_layout(framebuffer->width, framebuffer->height);
    chess_board_view_square_center(layout, animation.move.from, perspective, &from_x, &from_y);
    chess_board_view_square_center(layout, animation.move.to, perspective, &to_x, &to_y);

    t = animation.duration_ms <= 0.0f ? 1.0f : animation.elapsed_ms / animation.duration_ms;
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }
    visual_t = 1.0f - (1.0f - t) * (1.0f - t);
    x = (int)(from_x + (to_x - from_x) * visual_t);
    y = (int)(from_y + (to_y - from_y) * visual_t);

    chess_piece_view_render(framebuffer, animation.piece, x, y, layout.squareSize);
}
