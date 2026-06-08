#ifndef CAT_CHESS_JSON_H
#define CAT_CHESS_JSON_H

#include "cat_chess_models.h"

int cat_chess_json_get_int(const char* json, const char* key, int* out_value);
int cat_chess_json_get_long(const char* json, const char* key, long* out_value);
int cat_chess_json_get_string(const char* json, const char* key, char* out_value, int out_value_size);
int cat_chess_json_get_error(const char* json, char* out_error, int out_error_size);
int cat_chess_json_parse_device_secret(const char* json, char* out_secret, int out_secret_size);
int cat_chess_json_parse_game(const char* json, CatChessGameDto* out_game);
int cat_chess_json_parse_move(const char* json, CatChessMoveDto* out_move);
int cat_chess_json_parse_games(const char* json, CatChessGameListDto* out_games);
int cat_chess_json_parse_moves(const char* json, CatChessMoveListDto* out_moves);
int cat_chess_json_parse_http_status(const char* json, int* out_status);
int cat_chess_json_parse_http_body(const char* json, char* out_body, int out_body_size);

#endif
