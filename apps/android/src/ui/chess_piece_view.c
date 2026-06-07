#include "chess_piece_view.h"

#include "../sprites/generated_sprite.h"

static const char* chess_piece_sprite_id(ChessPieceType type) {
    switch (type) {
        case CHESS_PIECE_PAWN:
            return "piece_pawn";
        case CHESS_PIECE_KNIGHT:
            return "piece_knight";
        case CHESS_PIECE_BISHOP:
            return "piece_bishop";
        case CHESS_PIECE_ROOK:
            return "piece_rook";
        case CHESS_PIECE_QUEEN:
            return "piece_queen";
        case CHESS_PIECE_KING:
            return "piece_king";
        case CHESS_PIECE_NONE:
        default:
            return 0;
    }
}

void chess_piece_view_render(
        Framebuffer* framebuffer,
        ChessPiece piece,
        int square_x,
        int square_y,
        int square_size
) {
    const GeneratedSprite* sprite;
    const char* sprite_id;
    int padding;
    uint32_t backing_color;
    uint32_t base_color;

    if (piece.type == CHESS_PIECE_NONE) {
        return;
    }

    sprite_id = chess_piece_sprite_id(piece.type);
    sprite = generated_sprite_get_by_id(sprite_id);
    if (sprite == 0) {
        return;
    }

    padding = square_size / 7;
    if (piece.color == CHESS_COLOR_WHITE) {
        backing_color = 0x000000ffu;
        base_color = 0xffffffffu;
    } else {
        backing_color = 0xffffffffu;
        base_color = 0x000000ffu;
    }

    renderer_draw_generated_sprite_palette(
            framebuffer,
            sprite,
            square_x + padding,
            square_y + padding,
            square_size - padding * 2,
            square_size - padding * 2,
            backing_color,
            base_color
    );
}
