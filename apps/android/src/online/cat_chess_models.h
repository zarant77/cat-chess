#ifndef CAT_CHESS_MODELS_H
#define CAT_CHESS_MODELS_H

#define CAT_CHESS_BASE_URL_ANDROID_EMULATOR "http://10.0.2.2:5400"
#define CAT_CHESS_DEVICE_SECRET_MAX 64
#define CAT_CHESS_INVITE_CODE_MAX 16
#define CAT_CHESS_BOARD_FEN_MAX 128
#define CAT_CHESS_UCI_MAX 8
#define CAT_CHESS_API_ERROR_CODE_MAX 64
#define CAT_CHESS_GAME_LIST_MAX 16
#define CAT_CHESS_MOVE_LIST_MAX 64

typedef enum {
    CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK = 0,
    CAT_CHESS_GAME_STATUS_ACTIVE,
    CAT_CHESS_GAME_STATUS_FINISHED,
    CAT_CHESS_GAME_STATUS_UNKNOWN
} CatChessGameStatus;

typedef enum {
    CAT_CHESS_GAME_RESULT_NONE = 0,
    CAT_CHESS_GAME_RESULT_WHITE_WON,
    CAT_CHESS_GAME_RESULT_BLACK_WON,
    CAT_CHESS_GAME_RESULT_DRAW,
    CAT_CHESS_GAME_RESULT_UNKNOWN
} CatChessGameResult;

typedef enum {
    CAT_CHESS_PLAYER_COLOR_WHITE = 0,
    CAT_CHESS_PLAYER_COLOR_BLACK,
    CAT_CHESS_PLAYER_COLOR_NONE
} CatChessPlayerColor;

typedef struct {
    int id;
    char invite_code[CAT_CHESS_INVITE_CODE_MAX];
    CatChessGameStatus status;
    CatChessGameResult result;
    char board_fen[CAT_CHESS_BOARD_FEN_MAX];
    CatChessPlayerColor side_to_move;
    CatChessPlayerColor your_color;
    long created_at;
    long updated_at;
    long started_at;
    long finished_at;
} CatChessGameDto;

typedef struct {
    int id;
    int game_id;
    int move_index;
    CatChessPlayerColor color;
    char uci[CAT_CHESS_UCI_MAX];
    char fen_after[CAT_CHESS_BOARD_FEN_MAX];
    long created_at;
} CatChessMoveDto;

void cat_chess_game_dto_init(CatChessGameDto* game);
void cat_chess_move_dto_init(CatChessMoveDto* move);
const char* cat_chess_game_status_string(CatChessGameStatus status);
const char* cat_chess_game_result_string(CatChessGameResult result);
const char* cat_chess_player_color_string(CatChessPlayerColor color);

#endif
