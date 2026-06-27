#include "sprites_packed.h"

const uint32_t SPRITE_PALETTE[] = {
    0xffffffff,
    0x000000ff,
    0x101722ee,
    0x81b29aff,
    0xf5eddbff,
    0x101218ff,
    0x58698cff,
    0x374158ff,
    0x85a1ddff,
    0xcf142bff,
    0x101722ff,
    0x0057b7ff,
    0xffd700ff,
    0x263244ff,
    0xf9f1e1ff,
    0xf4c542ff,
    0x3d405bff,
    0x4e6a82ff,
    0x203140ff,
    0x172231ee,
};

const uint16_t SPRITE_PALETTE_COUNT = 20;

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

static const PackedSpriteCommand ARROW_LEFT_COMMANDS[] = {
    { 0, 55, 46, 110, 92, 0, 2, 0 },
    { 0, 55, 2, 110, 4, 0, 3, 0 },
    { 0, 55, 90, 110, 4, 0, 3, 0 },
    { 0, 2, 46, 4, 92, 0, 3, 0 },
    { 0, 108, 46, 4, 92, 0, 3, 0 },
    { 2, 59, 50, 43, 58, 191, 1, 0 },
    { 2, 53, 46, 43, 58, 191, 4, 0 },
    { 2, 62, 46, 24, 35, 191, 3, 0 },
};

static const PackedSpriteDefinition ARROW_LEFT_SPRITE = {
    .id = "arrow_left",

    .width = 110,
    .height = 92,
    .pivot_x = 55,
    .pivot_y = 46,

    .commands = ARROW_LEFT_COMMANDS,
    .command_count = 8,
};

static const PackedSpriteCommand ARROW_RIGHT_COMMANDS[] = {
    { 0, 55, 46, 110, 92, 0, 2, 0 },
    { 0, 55, 2, 110, 4, 0, 3, 0 },
    { 0, 55, 90, 110, 4, 0, 3, 0 },
    { 0, 2, 46, 4, 92, 0, 3, 0 },
    { 0, 108, 46, 4, 92, 0, 3, 0 },
    { 2, 51, 50, 43, 58, 64, 1, 0 },
    { 2, 57, 46, 43, 58, 64, 4, 0 },
    { 2, 48, 46, 24, 35, 64, 3, 0 },
};

static const PackedSpriteDefinition ARROW_RIGHT_SPRITE = {
    .id = "arrow_right",

    .width = 110,
    .height = 92,
    .pivot_x = 55,
    .pivot_y = 46,

    .commands = ARROW_RIGHT_COMMANDS,
    .command_count = 8,
};

static const PackedSpriteCommand BTN_PAUSE_COMMANDS[] = {
    { 1, 24, 24, 46, 47, 0, 5, 0 },
    { 1, 104, 24, 46, 47, 0, 5, 0 },
    { 1, 104, 104, 46, 47, 0, 5, 0 },
    { 1, 24, 104, 46, 47, 0, 5, 0 },
    { 0, 8, 65, 14, 85, 0, 5, 0 },
    { 0, 120, 63, 14, 85, 0, 5, 0 },
    { 0, 64, 8, 74, 14, 0, 5, 0 },
    { 0, 66, 120, 74, 14, 0, 5, 0 },
    { 1, 24, 24, 33, 34, 0, 6, 0 },
    { 1, 104, 24, 33, 34, 0, 6, 0 },
    { 1, 24, 104, 33, 34, 0, 6, 0 },
    { 1, 104, 104, 33, 34, 0, 6, 0 },
    { 0, 65, 19, 82, 24, 0, 6, 0 },
    { 0, 64, 109, 82, 24, 0, 6, 0 },
    { 0, 22, 64, 28, 82, 0, 6, 0 },
    { 0, 107, 63, 28, 82, 0, 6, 0 },
    { 1, 24, 24, 20, 20, 0, 1, 0 },
    { 1, 24, 105, 20, 20, 0, 1, 0 },
    { 1, 104, 24, 20, 20, 0, 1, 0 },
    { 1, 104, 105, 20, 20, 0, 1, 0 },
    { 0, 64, 24, 82, 20, 0, 5, 0 },
    { 0, 63, 105, 82, 20, 0, 5, 0 },
    { 0, 24, 65, 20, 80, 0, 5, 0 },
    { 0, 104, 63, 20, 80, 0, 5, 0 },
    { 1, 25, 25, 14, 14, 0, 7, 0 },
    { 1, 103, 25, 14, 14, 0, 7, 0 },
    { 0, 65, 23, 80, 10, 0, 7, 0 },
    { 1, 25, 104, 14, 14, 0, 7, 0 },
    { 1, 103, 104, 14, 14, 0, 7, 0 },
    { 0, 63, 106, 80, 10, 0, 7, 0 },
    { 0, 23, 65, 10, 80, 0, 7, 0 },
    { 0, 105, 64, 10, 80, 0, 7, 0 },
    { 0, 64, 64, 80, 80, 0, 7, 0 },
    { 1, 49, 38, 8, 8, 0, 1, 0 },
    { 1, 55, 38, 8, 8, 0, 1, 0 },
    { 0, 52, 38, 7, 7, 0, 1, 0 },
    { 1, 49, 90, 8, 8, 0, 1, 0 },
    { 1, 55, 90, 8, 8, 0, 1, 0 },
    { 0, 52, 90, 7, 7, 0, 1, 0 },
    { 0, 52, 64, 14, 51, 0, 1, 0 },
    { 1, 50, 39, 6, 6, 0, 4, 0 },
    { 1, 54, 39, 6, 6, 0, 4, 0 },
    { 1, 50, 89, 6, 6, 0, 4, 0 },
    { 1, 54, 89, 6, 6, 0, 4, 0 },
    { 0, 52, 64, 10, 50, 0, 4, 0 },
    { 0, 52, 64, 4, 56, 0, 4, 0 },
    { 1, 75, 38, 8, 8, 0, 1, 0 },
    { 1, 81, 38, 8, 8, 0, 1, 0 },
    { 0, 78, 38, 7, 7, 0, 1, 0 },
    { 1, 75, 90, 8, 8, 0, 1, 0 },
    { 1, 81, 90, 8, 8, 0, 1, 0 },
    { 0, 78, 90, 7, 7, 0, 1, 0 },
    { 0, 78, 64, 14, 51, 0, 1, 0 },
    { 1, 76, 39, 6, 6, 0, 4, 0 },
    { 1, 80, 39, 6, 6, 0, 4, 0 },
    { 1, 76, 89, 6, 6, 0, 4, 0 },
    { 1, 80, 89, 6, 6, 0, 4, 0 },
    { 0, 78, 64, 10, 50, 0, 4, 0 },
    { 0, 78, 64, 4, 56, 0, 4, 0 },
    { 2, 10, 50, 6, 4, 61, 8, 0 },
    { 2, 9, 48, 6, 9, 80, 1, 0 },
    { 2, 83, 116, 5, 6, 146, 1, 0 },
    { 2, 54, 9, 7, 8, 114, 1, 0 },
    { 2, 120, 86, 11, 5, 200, 1, 0 },
    { 2, 119, 42, 5, 6, 193, 1, 0 },
};

static const PackedSpriteDefinition BTN_PAUSE_SPRITE = {
    .id = "btn_pause",

    .width = 128,
    .height = 128,
    .pivot_x = 64,
    .pivot_y = 64,

    .commands = BTN_PAUSE_COMMANDS,
    .command_count = 65,
};

static const PackedSpriteCommand FLAG_EN_COMMANDS[] = {
    { 0, 48, 32, 88, 58, 0, 0, 0 },
    { 0, 48, 32, 14, 56, 0, 9, 0 },
    { 0, 48, 32, 88, 14, 0, 9, 0 },
    { 0, 47, 2, 96, 4, 0, 10, 0 },
    { 0, 48, 62, 96, 4, 0, 10, 0 },
    { 0, 2, 32, 4, 64, 0, 10, 0 },
    { 0, 94, 32, 4, 64, 0, 10, 0 },
};

static const PackedSpriteDefinition FLAG_EN_SPRITE = {
    .id = "flag_en",

    .width = 96,
    .height = 64,
    .pivot_x = 48,
    .pivot_y = 32,

    .commands = FLAG_EN_COMMANDS,
    .command_count = 7,
};

static const PackedSpriteCommand FLAG_UK_COMMANDS[] = {
    { 0, 48, 18, 88, 28, 0, 11, 0 },
    { 0, 48, 46, 88, 28, 0, 12, 0 },
    { 0, 47, 2, 96, 4, 0, 10, 0 },
    { 0, 48, 62, 96, 4, 0, 10, 0 },
    { 0, 2, 32, 4, 64, 0, 10, 0 },
    { 0, 94, 32, 4, 64, 0, 10, 0 },
};

static const PackedSpriteDefinition FLAG_UK_SPRITE = {
    .id = "flag_uk",

    .width = 96,
    .height = 64,
    .pivot_x = 48,
    .pivot_y = 32,

    .commands = FLAG_UK_COMMANDS,
    .command_count = 6,
};

static const PackedSpriteCommand MAIN_LOGO_COMMANDS[] = {
    { 0, 128, 72, 132, 54, 0, 13, 0 },
    { 0, 128, 78, 118, 38, 0, 14, 0 },
    { 2, 80, 39, 38, 42, 159, 13, 0 },
    { 2, 176, 39, 38, 42, 23, 13, 0 },
    { 1, 102, 62, 12, 12, 0, 15, 0 },
    { 1, 154, 62, 12, 12, 0, 15, 0 },
    { 0, 128, 82, 18, 6, 0, 13, 0 },
    { 0, 92, 91, 36, 5, 174, 13, 0 },
    { 0, 164, 91, 36, 5, 7, 13, 0 },
    { 0, 128, 110, 146, 8, 0, 3, 0 },
    { 0, 64, 110, 14, 24, 0, 13, 0 },
    { 0, 192, 110, 14, 24, 0, 13, 0 },
    { 0, 128, 24, 178, 10, 0, 16, 0 },
};

static const PackedSpriteDefinition MAIN_LOGO_SPRITE = {
    .id = "main-logo",

    .width = 256,
    .height = 128,
    .pivot_x = 128,
    .pivot_y = 64,

    .commands = MAIN_LOGO_COMMANDS,
    .command_count = 13,
};

static const PackedSpriteCommand PANEL_BG_COMMANDS[] = {
    { 1, 24, 24, 48, 48, 0, 2, 0 },
    { 1, 72, 24, 48, 48, 0, 2, 0 },
    { 1, 24, 72, 48, 48, 0, 2, 0 },
    { 1, 72, 72, 48, 48, 0, 2, 0 },
    { 0, 48, 24, 48, 48, 0, 2, 0 },
    { 0, 48, 72, 48, 48, 0, 2, 0 },
    { 0, 24, 48, 48, 48, 0, 2, 0 },
    { 0, 72, 48, 48, 48, 0, 2, 0 },
    { 0, 48, 48, 48, 48, 0, 2, 0 },
    { 0, 48, 8, 50, 6, 0, 17, 0 },
    { 0, 48, 88, 50, 6, 0, 18, 0 },
    { 0, 8, 48, 6, 50, 0, 17, 0 },
    { 0, 88, 48, 6, 50, 0, 18, 0 },
    { 1, 24, 24, 34, 34, 0, 17, 0 },
    { 1, 72, 24, 34, 34, 0, 17, 0 },
    { 1, 24, 72, 34, 34, 0, 18, 0 },
    { 1, 72, 72, 34, 34, 0, 18, 0 },
    { 1, 24, 24, 22, 22, 0, 2, 0 },
    { 1, 72, 24, 22, 22, 0, 2, 0 },
    { 1, 24, 72, 22, 22, 0, 2, 0 },
    { 1, 72, 72, 22, 22, 0, 2, 0 },
    { 0, 48, 48, 56, 56, 0, 19, 0 },
    { 0, 48, 18, 36, 3, 0, 3, 0 },
};

static const PackedSpriteDefinition PANEL_BG_SPRITE = {
    .id = "panel_bg",

    .width = 96,
    .height = 96,
    .pivot_x = 48,
    .pivot_y = 48,

    .commands = PANEL_BG_COMMANDS,
    .command_count = 23,
};

const PackedSpriteDefinition *PACKED_SPRITE_DEFINITIONS[] = {
    &PIECE_BISHOP_SPRITE,
    &PIECE_KING_SPRITE,
    &PIECE_KNIGHT_SPRITE,
    &PIECE_PAWN_SPRITE,
    &PIECE_QUEEN_SPRITE,
    &PIECE_ROOK_SPRITE,
    &ARROW_LEFT_SPRITE,
    &ARROW_RIGHT_SPRITE,
    &BTN_PAUSE_SPRITE,
    &FLAG_EN_SPRITE,
    &FLAG_UK_SPRITE,
    &MAIN_LOGO_SPRITE,
    &PANEL_BG_SPRITE,
};

const uint16_t PACKED_SPRITE_DEFINITION_COUNT = 13;
