#ifndef CAT_CHESS_GENERATED_SPRITE_H
#define CAT_CHESS_GENERATED_SPRITE_H

#include <stdint.h>

typedef enum
{
    SPRITE_NONE = -1,
    SPRITE_PIECE_BISHOP = 0,
    SPRITE_PIECE_KING,
    SPRITE_PIECE_KNIGHT,
    SPRITE_PIECE_PAWN,
    SPRITE_PIECE_QUEEN,
    SPRITE_PIECE_ROOK,
    SPRITE_ID_COUNT
} SpriteId;

typedef struct
{
    const char *id;
    int16_t width;
    int16_t height;
    int16_t pivot_x;
    int16_t pivot_y;
    uint32_t *pixels;
} GeneratedSprite;

void generated_sprite_initialize_all(void);
void generated_sprite_shutdown_all(void);

const GeneratedSprite *generated_sprite_get(SpriteId sprite_id);
const GeneratedSprite *generated_sprite_get_by_id(const char *sprite_id);

#endif
