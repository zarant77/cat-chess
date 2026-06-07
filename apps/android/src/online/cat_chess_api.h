#ifndef CAT_CHESS_API_H
#define CAT_CHESS_API_H

typedef enum {
    CAT_CHESS_API_OK = 0,
    CAT_CHESS_API_ERROR,
    CAT_CHESS_API_NOT_IMPLEMENTED
} CatChessApiResult;

CatChessApiResult cat_chess_api_create_device(void);
CatChessApiResult cat_chess_api_create_game(void);
CatChessApiResult cat_chess_api_join_game(const char* invite_code);
CatChessApiResult cat_chess_api_list_games(void);
CatChessApiResult cat_chess_api_get_game(int game_id);
CatChessApiResult cat_chess_api_make_move(int game_id, const char* uci);

#endif
