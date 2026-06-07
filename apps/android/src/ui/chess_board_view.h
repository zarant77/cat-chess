#ifndef CAT_CHESS_BOARD_VIEW_H
#define CAT_CHESS_BOARD_VIEW_H

#include "../chess/chess_game.h"
#include "../renderer/renderer.h"

typedef struct {
    int x;
    int y;
    int size;
    int squareSize;
} ChessBoardViewLayout;

ChessBoardViewLayout chess_board_view_layout(int screen_width, int screen_height);
int chess_board_view_square_at(int screen_width, int screen_height, float x, float y);
void chess_board_view_render(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square
);

#endif
