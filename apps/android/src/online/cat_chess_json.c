#include "cat_chess_json.h"

#include <ctype.h>
#include <string.h>

static const char* json_find_key(const char* json, const char* key) {
    char pattern[64];
    int key_length;

    if (json == 0 || key == 0) {
        return 0;
    }

    key_length = (int)strlen(key);
    if (key_length <= 0 || key_length > 56) {
        return 0;
    }

    pattern[0] = '"';
    memcpy(pattern + 1, key, (size_t)key_length);
    pattern[key_length + 1] = '"';
    pattern[key_length + 2] = '\0';

    return strstr(json, pattern);
}

static const char* json_value_start(const char* json, const char* key) {
    const char* cursor = json_find_key(json, key);

    if (cursor == 0) {
        return 0;
    }

    cursor = strchr(cursor, ':');
    if (cursor == 0) {
        return 0;
    }
    cursor += 1;

    while (*cursor != '\0' && isspace((unsigned char)*cursor)) {
        cursor += 1;
    }

    return cursor;
}

static int json_copy_string_at(const char* cursor, char* out_value, int out_value_size) {
    int written = 0;

    if (cursor == 0 || out_value == 0 || out_value_size <= 0 || *cursor != '"') {
        return 0;
    }

    cursor += 1;
    while (*cursor != '\0' && *cursor != '"') {
        char value = *cursor;

        if (value == '\\' && cursor[1] != '\0') {
            cursor += 1;
            value = *cursor;
            if (value == 'n') {
                value = '\n';
            } else if (value == 't') {
                value = '\t';
            }
        }

        if (written < out_value_size - 1) {
            out_value[written] = value;
            written += 1;
        }
        cursor += 1;
    }

    if (*cursor != '"') {
        out_value[0] = '\0';
        return 0;
    }

    out_value[written] = '\0';
    return 1;
}

int cat_chess_json_get_string(const char* json, const char* key, char* out_value, int out_value_size) {
    const char* cursor = json_value_start(json, key);

    if (out_value != 0 && out_value_size > 0) {
        out_value[0] = '\0';
    }

    if (cursor == 0) {
        return 0;
    }

    if (strncmp(cursor, "null", 4) == 0) {
        return 1;
    }

    return json_copy_string_at(cursor, out_value, out_value_size);
}

int cat_chess_json_get_int(const char* json, const char* key, int* out_value) {
    long value;

    if (!cat_chess_json_get_long(json, key, &value)) {
        return 0;
    }

    if (out_value != 0) {
        *out_value = (int)value;
    }
    return 1;
}

int cat_chess_json_get_long(const char* json, const char* key, long* out_value) {
    const char* cursor = json_value_start(json, key);
    long sign = 1;
    long value = 0;
    int has_digit = 0;

    if (out_value != 0) {
        *out_value = 0;
    }

    if (cursor == 0) {
        return 0;
    }

    if (strncmp(cursor, "null", 4) == 0) {
        return 1;
    }

    if (*cursor == '-') {
        sign = -1;
        cursor += 1;
    }

    while (*cursor >= '0' && *cursor <= '9') {
        has_digit = 1;
        value = value * 10 + (long)(*cursor - '0');
        cursor += 1;
    }

    if (!has_digit) {
        return 0;
    }

    if (out_value != 0) {
        *out_value = value * sign;
    }
    return 1;
}

static CatChessGameStatus json_status(const char* value) {
    if (strcmp(value, "waiting_for_black") == 0) {
        return CAT_CHESS_GAME_STATUS_WAITING_FOR_BLACK;
    }
    if (strcmp(value, "active") == 0) {
        return CAT_CHESS_GAME_STATUS_ACTIVE;
    }
    if (strcmp(value, "finished") == 0) {
        return CAT_CHESS_GAME_STATUS_FINISHED;
    }
    return CAT_CHESS_GAME_STATUS_UNKNOWN;
}

static CatChessGameResult json_result(const char* value) {
    if (value[0] == '\0') {
        return CAT_CHESS_GAME_RESULT_NONE;
    }
    if (strcmp(value, "white_won") == 0) {
        return CAT_CHESS_GAME_RESULT_WHITE_WON;
    }
    if (strcmp(value, "black_won") == 0) {
        return CAT_CHESS_GAME_RESULT_BLACK_WON;
    }
    if (strcmp(value, "draw") == 0) {
        return CAT_CHESS_GAME_RESULT_DRAW;
    }
    return CAT_CHESS_GAME_RESULT_UNKNOWN;
}

static CatChessPlayerColor json_color(const char* value) {
    if (value[0] == '\0') {
        return CAT_CHESS_PLAYER_COLOR_NONE;
    }
    if (strcmp(value, "white") == 0) {
        return CAT_CHESS_PLAYER_COLOR_WHITE;
    }
    if (strcmp(value, "black") == 0) {
        return CAT_CHESS_PLAYER_COLOR_BLACK;
    }
    return CAT_CHESS_PLAYER_COLOR_UNKNOWN;
}

int cat_chess_json_get_error(const char* json, char* out_error, int out_error_size) {
    return cat_chess_json_get_string(json, "error", out_error, out_error_size);
}

int cat_chess_json_parse_device_secret(const char* json, char* out_secret, int out_secret_size) {
    return cat_chess_json_get_string(json, "deviceSecret", out_secret, out_secret_size)
            && out_secret != 0
            && out_secret[0] != '\0';
}

int cat_chess_json_parse_game(const char* json, CatChessGameDto* out_game) {
    char status[32];
    char result[32];
    char side_to_move[16];
    char your_color[16];

    if (json == 0 || out_game == 0) {
        return 0;
    }

    cat_chess_game_dto_init(out_game);
    if (!cat_chess_json_get_int(json, "id", &out_game->id)) {
        return 0;
    }

    cat_chess_json_get_string(json, "inviteCode", out_game->invite_code, sizeof(out_game->invite_code));
    if (!cat_chess_json_get_string(json, "status", status, sizeof(status))) {
        return 0;
    }
    cat_chess_json_get_string(json, "result", result, sizeof(result));
    if (!cat_chess_json_get_string(json, "boardFen", out_game->board_fen, sizeof(out_game->board_fen))) {
        return 0;
    }
    if (!cat_chess_json_get_string(json, "sideToMove", side_to_move, sizeof(side_to_move))) {
        return 0;
    }
    cat_chess_json_get_string(json, "yourColor", your_color, sizeof(your_color));

    out_game->status = json_status(status);
    out_game->result = json_result(result);
    out_game->side_to_move = json_color(side_to_move);
    out_game->your_color = json_color(your_color);
    cat_chess_json_get_long(json, "createdAt", &out_game->created_at);
    cat_chess_json_get_long(json, "updatedAt", &out_game->updated_at);
    cat_chess_json_get_long(json, "startedAt", &out_game->started_at);
    cat_chess_json_get_long(json, "finishedAt", &out_game->finished_at);
    return 1;
}

int cat_chess_json_parse_move(const char* json, CatChessMoveDto* out_move) {
    char color[16];

    if (json == 0 || out_move == 0) {
        return 0;
    }

    cat_chess_move_dto_init(out_move);
    if (!cat_chess_json_get_int(json, "id", &out_move->id)
            || !cat_chess_json_get_int(json, "gameId", &out_move->game_id)
            || !cat_chess_json_get_int(json, "moveIndex", &out_move->move_index)
            || !cat_chess_json_get_string(json, "color", color, sizeof(color))
            || !cat_chess_json_get_string(json, "uci", out_move->uci, sizeof(out_move->uci))
            || !cat_chess_json_get_string(json, "fenAfter", out_move->fen_after, sizeof(out_move->fen_after))) {
        return 0;
    }

    out_move->color = json_color(color);
    cat_chess_json_get_long(json, "createdAt", &out_move->created_at);
    return 1;
}

static const char* json_array_start(const char* json, const char* key) {
    const char* cursor = json_value_start(json, key);

    if (cursor == 0 || *cursor != '[') {
        return 0;
    }

    return cursor + 1;
}

static const char* json_next_object(const char* cursor, char* out_object, int out_object_size) {
    int depth = 0;
    int in_string = 0;
    int written = 0;

    if (cursor == 0 || out_object == 0 || out_object_size <= 0) {
        return 0;
    }

    while (*cursor != '\0' && *cursor != '{' && *cursor != ']') {
        cursor += 1;
    }
    if (*cursor != '{') {
        return 0;
    }

    while (*cursor != '\0') {
        char value = *cursor;

        if (written < out_object_size - 1) {
            out_object[written] = value;
            written += 1;
        }

        if (value == '"' && (cursor == 0 || cursor[-1] != '\\')) {
            in_string = !in_string;
        } else if (!in_string) {
            if (value == '{') {
                depth += 1;
            } else if (value == '}') {
                depth -= 1;
                if (depth == 0) {
                    cursor += 1;
                    break;
                }
            }
        }
        cursor += 1;
    }

    out_object[written] = '\0';
    return cursor;
}

int cat_chess_json_parse_games(const char* json, CatChessGameListDto* out_games) {
    const char* cursor;
    char object[512];

    if (json == 0 || out_games == 0) {
        return 0;
    }

    cat_chess_game_list_dto_init(out_games);
    cursor = json_array_start(json, "games");
    if (cursor == 0) {
        return 0;
    }

    while (out_games->count < CAT_CHESS_MAX_GAMES) {
        cursor = json_next_object(cursor, object, sizeof(object));
        if (cursor == 0) {
            break;
        }
        if (!cat_chess_json_parse_game(object, out_games->items + out_games->count)) {
            return 0;
        }
        out_games->count += 1;
    }
    return 1;
}

int cat_chess_json_parse_moves(const char* json, CatChessMoveListDto* out_moves) {
    const char* cursor;
    char object[512];

    if (json == 0 || out_moves == 0) {
        return 0;
    }

    cat_chess_move_list_dto_init(out_moves);
    cursor = json_array_start(json, "moves");
    if (cursor == 0) {
        return 0;
    }

    while (out_moves->count < CAT_CHESS_MAX_MOVES) {
        cursor = json_next_object(cursor, object, sizeof(object));
        if (cursor == 0) {
            break;
        }
        if (!cat_chess_json_parse_move(object, out_moves->items + out_moves->count)) {
            return 0;
        }
        out_moves->count += 1;
    }
    return 1;
}

int cat_chess_json_parse_http_status(const char* json, int* out_status) {
    return cat_chess_json_get_int(json, "status", out_status);
}

int cat_chess_json_parse_http_body(const char* json, char* out_body, int out_body_size) {
    return cat_chess_json_get_string(json, "body", out_body, out_body_size);
}
