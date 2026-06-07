#include "sprites_packed.h"

const uint32_t SPRITE_PALETTE[] = {
    0xffffffff,
    0x000000ff,
};

const uint16_t SPRITE_PALETTE_COUNT = 2;

static const PackedSpriteCommand PIECE_BISHOP_COMMANDS[] = {
    { 1, 128, 48, 52, 52, 0, 0, 0 },
    { 1, 128, 112, 112, 144, 0, 0, 0 },
    { 0, 128, 188, 120, 48, 0, 0, 0 },
    { 0, 128, 216, 164, 36, 0, 0, 0 },
    { 1, 128, 50, 32, 32, 0, 1, 0 },
    { 1, 128, 114, 88, 120, 0, 1, 0 },
    { 0, 128, 188, 96, 32, 0, 1, 0 },
    { 0, 128, 216, 136, 20, 0, 1, 0 },
    { 0, 140, 108, 16, 84, 23, 0, 0 },
};

static const PackedSpriteDefinition PIECE_BISHOP_SPRITE = {
    .id = "piece_bishop",

    .width = 256,
    .height = 256,
    .pivot_x = 128,
    .pivot_y = 128,

    .commands = PIECE_BISHOP_COMMANDS,
    .command_count = 9,
};

static const PackedSpriteCommand PIECE_KING_COMMANDS[] = {
    { 0, 128, 210, 172, 40, 0, 0, 0 },
    { 1, 128, 156, 128, 104, 0, 0, 0 },
    { 0, 128, 104, 84, 116, 0, 0, 0 },
    { 0, 128, 40, 28, 64, 0, 0, 0 },
    { 0, 128, 36, 76, 24, 0, 0, 0 },
    { 0, 128, 210, 140, 24, 0, 1, 0 },
    { 1, 128, 156, 104, 84, 0, 1, 0 },
    { 0, 128, 104, 60, 92, 0, 1, 0 },
    { 0, 128, 42, 16, 48, 0, 1, 0 },
    { 0, 128, 38, 56, 12, 0, 1, 0 },
};

static const PackedSpriteDefinition PIECE_KING_SPRITE = {
    .id = "piece_king",

    .width = 256,
    .height = 256,
    .pivot_x = 128,
    .pivot_y = 128,

    .commands = PIECE_KING_COMMANDS,
    .command_count = 10,
};

static const PackedSpriteCommand PIECE_KNIGHT_COMMANDS[] = {
    { 0, 132, 210, 164, 40, 0, 0, 0 },
    { 0, 136, 160, 112, 108, 248, 0, 0 },
    { 1, 110, 104, 104, 116, 0, 0, 0 },
    { 2, 164, 80, 124, 96, 12, 0, 0 },
    { 2, 80, 56, 56, 68, 235, 0, 0 },
    { 0, 132, 210, 136, 24, 0, 1, 0 },
    { 0, 136, 160, 84, 88, 248, 1, 0 },
    { 1, 110, 104, 80, 92, 0, 1, 0 },
    { 2, 160, 84, 96, 72, 12, 1, 0 },
    { 2, 84, 60, 36, 48, 235, 1, 0 },
    { 1, 140, 88, 16, 16, 0, 0, 0 },
};

static const PackedSpriteDefinition PIECE_KNIGHT_SPRITE = {
    .id = "piece_knight",

    .width = 256,
    .height = 256,
    .pivot_x = 128,
    .pivot_y = 128,

    .commands = PIECE_KNIGHT_COMMANDS,
    .command_count = 11,
};

static const PackedSpriteCommand PIECE_PAWN_COMMANDS[] = {
    { 2, 113, 155, 52, 68, 0, 0, 0 },
    { 2, 145, 155, 52, 68, 0, 0, 0 },
    { 0, 128, 148, 46, 58, 0, 0, 0 },
    { 0, 128, 190, 90, 12, 0, 0, 0 },
    { 1, 85, 190, 16, 16, 0, 0, 0 },
    { 1, 171, 190, 16, 16, 0, 0, 0 },
    { 1, 128, 91, 78, 76, 0, 0, 0 },
    { 2, 105, 55, 42, 48, 239, 0, 0 },
    { 2, 151, 55, 42, 48, 16, 0, 0 },
    { 1, 128, 123, 50, 24, 0, 0, 0 },
    { 1, 128, 91, 64, 62, 0, 1, 0 },
    { 2, 106, 58, 28, 34, 239, 1, 0 },
    { 2, 150, 58, 28, 34, 16, 1, 0 },
    { 1, 116, 91, 8, 8, 0, 0, 0 },
    { 1, 140, 91, 8, 8, 0, 0, 0 },
    { 2, 128, 103, 12, 10, 128, 0, 0 },
    { 1, 111, 123, 16, 16, 0, 0, 0 },
    { 1, 145, 123, 16, 16, 0, 0, 0 },
    { 0, 128, 123, 38, 14, 0, 0, 0 },
    { 1, 111, 123, 8, 8, 0, 1, 0 },
    { 1, 145, 123, 8, 8, 0, 1, 0 },
    { 0, 128, 123, 34, 6, 0, 1, 0 },
    { 2, 113, 157, 38, 54, 0, 1, 0 },
    { 2, 145, 157, 38, 54, 0, 1, 0 },
    { 0, 128, 150, 32, 46, 0, 1, 0 },
    { 0, 128, 190, 76, 6, 0, 1, 0 },
};

static const PackedSpriteDefinition PIECE_PAWN_SPRITE = {
    .id = "piece_pawn",

    .width = 256,
    .height = 256,
    .pivot_x = 128,
    .pivot_y = 128,

    .commands = PIECE_PAWN_COMMANDS,
    .command_count = 26,
};

static const PackedSpriteCommand PIECE_QUEEN_COMMANDS[] = {
    { 0, 128, 210, 172, 40, 0, 0, 0 },
    { 1, 128, 156, 128, 100, 0, 0, 0 },
    { 2, 80, 100, 72, 124, 248, 0, 0 },
    { 2, 128, 88, 80, 144, 0, 0, 0 },
    { 2, 176, 100, 72, 124, 7, 0, 0 },
    { 1, 76, 52, 36, 36, 0, 0, 0 },
    { 1, 128, 40, 40, 40, 0, 0, 0 },
    { 1, 180, 52, 36, 36, 0, 0, 0 },
    { 0, 128, 210, 140, 24, 0, 1, 0 },
    { 1, 128, 156, 104, 80, 0, 1, 0 },
    { 2, 84, 104, 48, 100, 248, 1, 0 },
    { 2, 128, 92, 56, 116, 0, 1, 0 },
    { 2, 172, 104, 48, 100, 7, 1, 0 },
    { 1, 76, 54, 20, 20, 0, 1, 0 },
    { 1, 128, 42, 24, 24, 0, 1, 0 },
    { 1, 180, 54, 20, 20, 0, 1, 0 },
};

static const PackedSpriteDefinition PIECE_QUEEN_SPRITE = {
    .id = "piece_queen",

    .width = 256,
    .height = 256,
    .pivot_x = 128,
    .pivot_y = 128,

    .commands = PIECE_QUEEN_COMMANDS,
    .command_count = 16,
};

static const PackedSpriteCommand PIECE_ROOK_COMMANDS[] = {
    { 0, 128, 210, 168, 40, 0, 0, 0 },
    { 0, 128, 160, 120, 108, 0, 0, 0 },
    { 0, 128, 100, 144, 60, 0, 0, 0 },
    { 0, 84, 62, 36, 56, 0, 0, 0 },
    { 0, 128, 62, 36, 56, 0, 0, 0 },
    { 0, 172, 62, 36, 56, 0, 0, 0 },
    { 0, 128, 210, 140, 24, 0, 1, 0 },
    { 0, 128, 160, 96, 88, 0, 1, 0 },
    { 0, 128, 100, 120, 40, 0, 1, 0 },
    { 0, 84, 64, 20, 40, 0, 1, 0 },
    { 0, 128, 64, 20, 40, 0, 1, 0 },
    { 0, 172, 64, 20, 40, 0, 1, 0 },
};

static const PackedSpriteDefinition PIECE_ROOK_SPRITE = {
    .id = "piece_rook",

    .width = 256,
    .height = 256,
    .pivot_x = 128,
    .pivot_y = 128,

    .commands = PIECE_ROOK_COMMANDS,
    .command_count = 12,
};

const PackedSpriteDefinition *PACKED_SPRITE_DEFINITIONS[] = {
    &PIECE_BISHOP_SPRITE,
    &PIECE_KING_SPRITE,
    &PIECE_KNIGHT_SPRITE,
    &PIECE_PAWN_SPRITE,
    &PIECE_QUEEN_SPRITE,
    &PIECE_ROOK_SPRITE,
};

const uint16_t PACKED_SPRITE_DEFINITION_COUNT = 6;
