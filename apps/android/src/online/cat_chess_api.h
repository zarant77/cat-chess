#ifndef CAT_CHESS_API_H
#define CAT_CHESS_API_H

#include "cat_chess_models.h"

typedef enum {
    CAT_CHESS_API_OK = 0,
    CAT_CHESS_API_ERROR,
    CAT_CHESS_API_NOT_IMPLEMENTED,
    CAT_CHESS_API_UNAUTHORIZED,
    CAT_CHESS_API_NOT_FOUND,
    CAT_CHESS_API_CONFLICT
} CatChessApiResult;

typedef struct {
    CatChessApiResult result;
    char error_code[CAT_CHESS_API_ERROR_CODE_MAX];
} CatChessApiResponse;

typedef struct {
    char base_url[128];
    char device_secret[CAT_CHESS_DEVICE_SECRET_MAX];
    CatChessApiResponse last_response;
} CatChessApiClient;

void cat_chess_api_init(CatChessApiClient* client, const char* base_url);
void cat_chess_api_set_device_secret(CatChessApiClient* client, const char* device_secret);
int cat_chess_api_is_uci_move(const char* uci);
void cat_chess_api_build_uci(int from_square, int to_square, char promotion, char* out_uci, int out_uci_size);

CatChessApiResult cat_chess_api_create_or_touch_device(
        CatChessApiClient* client,
        char* out_device_secret,
        int out_device_secret_size
);

CatChessApiResult cat_chess_api_create_game(
        CatChessApiClient* client,
        CatChessGameDto* out_game
);

CatChessApiResult cat_chess_api_join_game(
        CatChessApiClient* client,
        const char* invite_code,
        CatChessGameDto* out_game
);

CatChessApiResult cat_chess_api_list_games(
        CatChessApiClient* client,
        CatChessGameDto* out_games,
        int max_games,
        int* out_game_count
);

CatChessApiResult cat_chess_api_get_game(
        CatChessApiClient* client,
        int game_id,
        CatChessGameDto* out_game
);

CatChessApiResult cat_chess_api_get_moves(
        CatChessApiClient* client,
        int game_id,
        CatChessMoveDto* out_moves,
        int max_moves,
        int* out_move_count
);

CatChessApiResult cat_chess_api_make_move(
        CatChessApiClient* client,
        int game_id,
        const char* uci,
        CatChessGameDto* out_game
);

CatChessApiResult cat_chess_api_resign_game(
        CatChessApiClient* client,
        int game_id,
        CatChessGameDto* out_game
);

#endif
