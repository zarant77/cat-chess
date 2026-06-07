#include "cat_chess_api.h"

#include <string.h>

#include "../chess/chess_board.h"

static void cat_chess_api_clear_response(CatChessApiClient* client, CatChessApiResult result) {
    if (client == 0) {
        return;
    }

    client->last_response.result = result;
    client->last_response.error_code[0] = '\0';
}

static CatChessApiResult cat_chess_api_stub(CatChessApiClient* client) {
    cat_chess_api_clear_response(client, CAT_CHESS_API_NOT_IMPLEMENTED);
    if (client != 0) {
        strncpy(
                client->last_response.error_code,
                "network_not_implemented",
                sizeof(client->last_response.error_code) - 1u
        );
        client->last_response.error_code[sizeof(client->last_response.error_code) - 1u] = '\0';
    }
    return CAT_CHESS_API_NOT_IMPLEMENTED;
}

void cat_chess_api_init(CatChessApiClient* client, const char* base_url) {
    if (client == 0) {
        return;
    }

    client->base_url[0] = '\0';
    client->device_secret[0] = '\0';
    cat_chess_api_clear_response(client, CAT_CHESS_API_OK);

    if (base_url != 0 && base_url[0] != '\0') {
        strncpy(client->base_url, base_url, sizeof(client->base_url) - 1u);
        client->base_url[sizeof(client->base_url) - 1u] = '\0';
    }
}

void cat_chess_api_set_device_secret(CatChessApiClient* client, const char* device_secret) {
    if (client == 0) {
        return;
    }

    client->device_secret[0] = '\0';
    if (device_secret != 0 && device_secret[0] != '\0') {
        strncpy(client->device_secret, device_secret, sizeof(client->device_secret) - 1u);
        client->device_secret[sizeof(client->device_secret) - 1u] = '\0';
    }
}

int cat_chess_api_is_uci_move(const char* uci) {
    int length = 0;

    if (uci == 0) {
        return 0;
    }

    while (uci[length] != '\0') {
        length += 1;
    }

    if (length != 4 && length != 5) {
        return 0;
    }

    if (uci[0] < 'a' || uci[0] > 'h' || uci[2] < 'a' || uci[2] > 'h') {
        return 0;
    }
    if (uci[1] < '1' || uci[1] > '8' || uci[3] < '1' || uci[3] > '8') {
        return 0;
    }
    if (length == 5 && uci[4] != 'q' && uci[4] != 'r' && uci[4] != 'b' && uci[4] != 'n') {
        return 0;
    }

    return 1;
}

void cat_chess_api_build_uci(int from_square, int to_square, char promotion, char* out_uci, int out_uci_size) {
    int from_file = from_square % CHESS_BOARD_SIZE;
    int from_rank = from_square / CHESS_BOARD_SIZE;
    int to_file = to_square % CHESS_BOARD_SIZE;
    int to_rank = to_square / CHESS_BOARD_SIZE;
    int length = promotion == '\0' ? 4 : 5;

    if (out_uci == 0 || out_uci_size <= 0) {
        return;
    }

    out_uci[0] = '\0';
    if (out_uci_size <= length) {
        return;
    }

    if (from_square < 0
            || from_square >= CHESS_BOARD_SQUARE_COUNT
            || to_square < 0
            || to_square >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    if (promotion != '\0' && promotion != 'q' && promotion != 'r' && promotion != 'b' && promotion != 'n') {
        return;
    }

    out_uci[0] = (char)('a' + from_file);
    out_uci[1] = (char)('8' - from_rank);
    out_uci[2] = (char)('a' + to_file);
    out_uci[3] = (char)('8' - to_rank);
    if (promotion != '\0') {
        out_uci[4] = promotion;
    }
    out_uci[length] = '\0';
}

CatChessApiResult cat_chess_api_create_or_touch_device(
        CatChessApiClient* client,
        char* out_device_secret,
        int out_device_secret_size
) {
    (void)out_device_secret;
    (void)out_device_secret_size;
    return cat_chess_api_stub(client);
}

CatChessApiResult cat_chess_api_create_game(
        CatChessApiClient* client,
        CatChessGameDto* out_game
) {
    cat_chess_game_dto_init(out_game);
    return cat_chess_api_stub(client);
}

CatChessApiResult cat_chess_api_join_game(
        CatChessApiClient* client,
        const char* invite_code,
        CatChessGameDto* out_game
) {
    (void)invite_code;
    cat_chess_game_dto_init(out_game);
    return cat_chess_api_stub(client);
}

CatChessApiResult cat_chess_api_list_games(
        CatChessApiClient* client,
        CatChessGameDto* out_games,
        int max_games,
        int* out_game_count
) {
    if (out_game_count != 0) {
        *out_game_count = 0;
    }
    if (out_games != 0 && max_games > 0) {
        cat_chess_game_dto_init(out_games);
    }
    return cat_chess_api_stub(client);
}

CatChessApiResult cat_chess_api_get_game(
        CatChessApiClient* client,
        int game_id,
        CatChessGameDto* out_game
) {
    (void)game_id;
    cat_chess_game_dto_init(out_game);
    return cat_chess_api_stub(client);
}

CatChessApiResult cat_chess_api_get_moves(
        CatChessApiClient* client,
        int game_id,
        CatChessMoveDto* out_moves,
        int max_moves,
        int* out_move_count
) {
    (void)game_id;
    if (out_move_count != 0) {
        *out_move_count = 0;
    }
    if (out_moves != 0 && max_moves > 0) {
        cat_chess_move_dto_init(out_moves);
    }
    return cat_chess_api_stub(client);
}

CatChessApiResult cat_chess_api_make_move(
        CatChessApiClient* client,
        int game_id,
        const char* uci,
        CatChessGameDto* out_game
) {
    (void)game_id;
    (void)uci;
    cat_chess_game_dto_init(out_game);
    return cat_chess_api_stub(client);
}

CatChessApiResult cat_chess_api_resign_game(
        CatChessApiClient* client,
        int game_id,
        CatChessGameDto* out_game
) {
    (void)game_id;
    cat_chess_game_dto_init(out_game);
    return cat_chess_api_stub(client);
}
