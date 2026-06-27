#ifndef CAT_CHESS_BOARD_VIEW_H
#define CAT_CHESS_BOARD_VIEW_H

#include "../chess/chess_game.h"
#include "../chess/chess_rules.h"
#include "../online/online_game.h"
#include "../renderer/renderer.h"

typedef struct {
    int x;
    int y;
    int size;
    int squareSize;
} ChessBoardViewLayout;

ChessBoardViewLayout chess_board_view_layout(int screen_width, int screen_height);
int chess_board_view_square_at(int screen_width, int screen_height, float x, float y);
int chess_board_view_square_at_for_color(
        int screen_width,
        int screen_height,
        float x,
        float y,
        ChessColor perspective
);
void chess_board_view_render(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square
);
void chess_board_view_render_for_color(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square,
        ChessColor perspective
);
void chess_board_view_render_for_color_with_hidden_square(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square,
        ChessColor perspective,
        int hidden_square
);
void chess_board_view_render_for_color_with_hidden_square_and_hints(
        Framebuffer* framebuffer,
        const ChessGame* game,
        int selected_square,
        int last_tapped_square,
        ChessColor perspective,
        int hidden_square,
        const ChessMoveList* move_hints
);
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
);
void chess_board_view_render_animated_piece(
        Framebuffer* framebuffer,
        ChessMoveAnimation animation,
        ChessColor perspective
);

#endif
