#include "cat_chess_api.h"

CatChessApiResult cat_chess_api_create_device(void) {
    return CAT_CHESS_API_NOT_IMPLEMENTED;
}

CatChessApiResult cat_chess_api_create_game(void) {
    return CAT_CHESS_API_NOT_IMPLEMENTED;
}

CatChessApiResult cat_chess_api_join_game(const char* invite_code) {
    (void)invite_code;
    return CAT_CHESS_API_NOT_IMPLEMENTED;
}

CatChessApiResult cat_chess_api_list_games(void) {
    return CAT_CHESS_API_NOT_IMPLEMENTED;
}

CatChessApiResult cat_chess_api_get_game(int game_id) {
    (void)game_id;
    return CAT_CHESS_API_NOT_IMPLEMENTED;
}

CatChessApiResult cat_chess_api_make_move(int game_id, const char* uci) {
    (void)game_id;
    (void)uci;
    return CAT_CHESS_API_NOT_IMPLEMENTED;
}
