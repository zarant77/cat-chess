#ifndef CAT_CHESS_PIECE_VIEW_H
#define CAT_CHESS_PIECE_VIEW_H

#include "../chess/chess_board.h"
#include "../renderer/renderer.h"

void chess_piece_view_render(
        Framebuffer* framebuffer,
        ChessPiece piece,
        int square_x,
        int square_y,
        int square_size
);

#endif
