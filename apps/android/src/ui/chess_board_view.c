#include "chess_board_view.h"

#include "../ui/chess_piece_view.h"

static int min_int(int left, int right) {
    return left < right ? left : right;
}

ChessBoardViewLayout chess_board_view_layout(int screen_width, int screen_height) {
    ChessBoardViewLayout layout;
    int margin = 24;
    int available_width = screen_width - margin * 2;
    int available_height = screen_height - margin * 2;

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
    layout.y = (screen_height - layout.size) / 2;

    return layout;
}

int chess_board_view_square_at(int screen_width, int screen_height, float x, float y) {
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
    rank = ((int)y - layout.y) / layout.squareSize;

    return chess_board_square(file, rank);
}

static void chess_board_view_draw_border(Framebuffer* framebuffer, ChessBoardViewLayout layout) {
    renderer_draw_color_rect(framebuffer, layout.x - 5, layout.y - 5, layout.size + 10, 5, 0x223127ffu);
    renderer_draw_color_rect(framebuffer, layout.x - 5, layout.y + layout.size, layout.size + 10, 5, 0x223127ffu);
    renderer_draw_color_rect(framebuffer, layout.x - 5, layout.y, 5, layout.size, 0x223127ffu);
    renderer_draw_color_rect(framebuffer, layout.x + layout.size, layout.y, 5, layout.size, 0x223127ffu);
}

void chess_board_view_render(Framebuffer* framebuffer, const GameState* game) {
    ChessBoardViewLayout layout;

    if (framebuffer == 0 || game == 0) {
        return;
    }

    layout = chess_board_view_layout(framebuffer->width, framebuffer->height);
    chess_board_view_draw_border(framebuffer, layout);

    for (int rank = 0; rank < CHESS_BOARD_SIZE; ++rank) {
        for (int file = 0; file < CHESS_BOARD_SIZE; ++file) {
            int square = chess_board_square(file, rank);
            int x = layout.x + file * layout.squareSize;
            int y = layout.y + rank * layout.squareSize;
            uint32_t color = ((file + rank) & 1) ? 0x6d8c6affu : 0xe8d9b7ffu;

            renderer_draw_color_rect(framebuffer, x, y, layout.squareSize, layout.squareSize, color);

            if (game->hasSelection && square == game->selectedSquare) {
                renderer_draw_color_rect(framebuffer, x, y, layout.squareSize, 4, 0xf4c542ffu);
                renderer_draw_color_rect(framebuffer, x, y + layout.squareSize - 4, layout.squareSize, 4, 0xf4c542ffu);
                renderer_draw_color_rect(framebuffer, x, y, 4, layout.squareSize, 0xf4c542ffu);
                renderer_draw_color_rect(framebuffer, x + layout.squareSize - 4, y, 4, layout.squareSize, 0xf4c542ffu);
            } else if (square == game->tappedSquare) {
                renderer_draw_color_rect(framebuffer, x + 4, y + 4, layout.squareSize - 8, layout.squareSize - 8, 0xffffff33u);
            }

            chess_piece_view_render(
                    framebuffer,
                    chess_board_get_piece(&game->board, square),
                    x,
                    y,
                    layout.squareSize
            );
        }
    }
}
