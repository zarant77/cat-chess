#include "app.h"

#include <string.h>

#include "../audio/chess_sound.h"
#include "../chess/chess_ai.h"
#include "../chess/chess_rules.h"
#include "../online/cat_chess_api_config.h"
#include "../online/cat_chess_models.h"
#include "../storage/local_storage.h"
#include "../ui/chess_board_view.h"
#include "../ui/screens/screen_create_game.h"
#include "../ui/screens/screen_game.h"
#include "../ui/screens/screen_home.h"
#include "../ui/screens/screen_join_game.h"
#include "../ui/screens/screen_my_games.h"
#include "../ui/screens/screen_settings.h"
#include "../ui/screens/screen_local_game.h"

static void app_clear_selection(AppState* app) {
    app->selectedSquare = -1;
    app->hasSelection = 0;
}

static void app_start_local_game(AppState* app) {
    if (app == 0) {
        return;
    }

    chess_game_init(&app->localGame);
    app->localGameMessage = LOCAL_GAME_MESSAGE_YOUR_MOVE;
    app->localGameOver = 0;
    app->lastTappedSquare = -1;
    app_clear_selection(app);
}

static void app_update_local_game_message(AppState* app, ChessColor last_mover) {
    ChessGameStatus status;

    if (app == 0) {
        return;
    }

    status = chess_get_game_status(&app->localGame.board);
    if (status == CHESS_GAME_CHECKMATE) {
        app->localGameOver = 1;
        app->localGameMessage = last_mover == CHESS_COLOR_WHITE
                ? LOCAL_GAME_MESSAGE_YOU_WON
                : LOCAL_GAME_MESSAGE_YOU_LOST;
        return;
    }
    if (status == CHESS_GAME_STALEMATE) {
        app->localGameOver = 1;
        app->localGameMessage = LOCAL_GAME_MESSAGE_DRAW;
        return;
    }

    app->localGameOver = 0;
    app->localGameMessage = chess_is_in_check(&app->localGame.board, app->localGame.board.side_to_move)
            ? LOCAL_GAME_MESSAGE_CHECK
            : LOCAL_GAME_MESSAGE_YOUR_MOVE;
}

void app_init(AppState* app) {
    char device_secret[CAT_CHESS_DEVICE_SECRET_MAX];

    if (app == 0) {
        return;
    }

    app->currentScreen = APP_SCREEN_HOME;
    chess_game_init(&app->localGame);
    app->localGameMessage = LOCAL_GAME_MESSAGE_YOUR_MOVE;
    app->localGameOver = 0;
    game_settings_init(&app->settings);
    cat_chess_api_init(&app->apiClient, CAT_CHESS_API_BASE_URL);
    online_game_init(&app->onlineGame);
    cat_chess_game_list_dto_init(&app->games);
    app->lastApiStatus.result = CAT_CHESS_API_OK;
    app->lastApiStatus.http_status = 0;
    app->lastApiStatus.error_code[0] = '\0';
    app->inviteCode[0] = '\0';
    app->inviteInputFocused = 0;
    app->selectedSquare = -1;
    app->lastTappedSquare = -1;
    app->hasSelection = 0;
    if (local_storage_get_device_secret(device_secret, (int)sizeof(device_secret))) {
        cat_chess_api_set_device_secret(&app->apiClient, device_secret);
        app->deviceSecretStatus = DEVICE_SECRET_STATUS_AVAILABLE;
        app->networkStatus = NETWORK_STATUS_READY;
    } else {
        app->lastApiStatus = cat_chess_api_create_or_touch_device(
            &app->apiClient,
            device_secret,
            (int)sizeof(device_secret)
        );
        if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
            local_storage_set_device_secret(device_secret);
            cat_chess_api_set_device_secret(&app->apiClient, device_secret);
            app->deviceSecretStatus = DEVICE_SECRET_STATUS_AVAILABLE;
            app->networkStatus = NETWORK_STATUS_READY;
        } else {
            app->deviceSecretStatus = DEVICE_SECRET_STATUS_MISSING;
            app->networkStatus = NETWORK_STATUS_ERROR;
        }
    }
    app->screenWidth = 0;
    app->screenHeight = 0;
    app->fps = 0;
    app->averageFrameMs = 0;
    app->exitRequested = 0;
    app->softKeyboardRequested = 0;
    chess_sound_play(CHESS_SOUND_GAME_START);
}

void app_set_screen_size(AppState* app, float width, float height) {
    if (app == 0) {
        return;
    }

    app->screenWidth = (int)width;
    app->screenHeight = (int)height;
}

void app_navigate(AppState* app, AppScreen screen) {
    if (app == 0) {
        return;
    }

    app->currentScreen = screen;
    app_clear_selection(app);
    app->inviteInputFocused = 0;
    if (screen == APP_SCREEN_LOCAL_GAME) {
        app_start_local_game(app);
    } else if (screen == APP_SCREEN_GAME) {
        app->lastTappedSquare = -1;
    }
}

static int app_ensure_device_secret(AppState* app) {
    char device_secret[CAT_CHESS_DEVICE_SECRET_MAX];

    if (app == 0) {
        return 0;
    }

    if (app->apiClient.device_secret[0] != '\0') {
        return 1;
    }

    if (local_storage_get_device_secret(device_secret, (int)sizeof(device_secret))) {
        cat_chess_api_set_device_secret(&app->apiClient, device_secret);
        app->deviceSecretStatus = DEVICE_SECRET_STATUS_AVAILABLE;
        return 1;
    }

    app->lastApiStatus = cat_chess_api_create_or_touch_device(
            &app->apiClient,
            device_secret,
            (int)sizeof(device_secret)
    );
    if (app->lastApiStatus.result != CAT_CHESS_API_OK) {
        app->networkStatus = NETWORK_STATUS_ERROR;
        return 0;
    }

    local_storage_set_device_secret(device_secret);
    cat_chess_api_set_device_secret(&app->apiClient, device_secret);
    app->deviceSecretStatus = DEVICE_SECRET_STATUS_AVAILABLE;
    return 1;
}

void app_create_online_game(AppState* app) {
    if (app == 0 || !app_ensure_device_secret(app)) {
        return;
    }

    app->onlineGame.state = ONLINE_GAME_PLACEHOLDER_LOADING;
    app->lastApiStatus = cat_chess_api_create_game(&app->apiClient, &app->onlineGame.game);
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        app->onlineGame.state = ONLINE_GAME_PLACEHOLDER_IDLE;
        app_navigate(app, APP_SCREEN_GAME);
    } else {
        app->onlineGame.state = ONLINE_GAME_PLACEHOLDER_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
    }
}

void app_join_online_game(AppState* app) {
    if (app == 0 || app->inviteCode[0] == '\0' || !app_ensure_device_secret(app)) {
        return;
    }

    app->onlineGame.state = ONLINE_GAME_PLACEHOLDER_LOADING;
    app->lastApiStatus = cat_chess_api_join_game(&app->apiClient, app->inviteCode, &app->onlineGame.game);
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        app->onlineGame.state = ONLINE_GAME_PLACEHOLDER_IDLE;
        app_navigate(app, APP_SCREEN_GAME);
    } else {
        app->onlineGame.state = ONLINE_GAME_PLACEHOLDER_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
    }
}

void app_refresh_online_game(AppState* app) {
    if (app == 0 || app->onlineGame.game.id <= 0 || !app_ensure_device_secret(app)) {
        return;
    }

    app->lastApiStatus = cat_chess_api_get_game(&app->apiClient, app->onlineGame.game.id, &app->onlineGame.game);
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        app->lastApiStatus = cat_chess_api_get_moves(&app->apiClient, app->onlineGame.game.id, &app->onlineGame.moves);
    }
}

static void app_handle_invite_text(AppState* app, const InputState* input) {
    int length;

    if (app == 0 || input == 0 || !app->inviteInputFocused) {
        return;
    }

    length = (int)strlen(app->inviteCode);
    if (input->textBackspace && length > 0) {
        length -= 1;
        app->inviteCode[length] = '\0';
    }

    for (int index = 0; index < input->textCharCount; ++index) {
        char value = input->textChars[index];
        if (value >= 'a' && value <= 'z') {
            value = (char)(value - 'a' + 'A');
        }
        if (((value >= 'A' && value <= 'Z') || (value >= '0' && value <= '9'))
                && length < CAT_CHESS_INVITE_CODE_MAX - 1) {
            app->inviteCode[length] = value;
            length += 1;
            app->inviteCode[length] = '\0';
        }
    }
}

void app_request_back(AppState* app) {
    if (app == 0) {
        return;
    }

    if (app->currentScreen == APP_SCREEN_HOME) {
        app->exitRequested = 1;
        return;
    }

    app_navigate(app, APP_SCREEN_HOME);
}

void app_request_soft_keyboard(AppState* app) {
    if (app == 0) {
        return;
    }

    app->softKeyboardRequested = 1;
}

int app_take_soft_keyboard_request(AppState* app) {
    int requested;

    if (app == 0) {
        return 0;
    }

    requested = app->softKeyboardRequested;
    app->softKeyboardRequested = 0;
    return requested;
}

static void app_handle_local_game_tap(AppState* app, int x, int y) {
    int square;
    ChessPiece piece;
    ChessPiece moving;
    ChessPiece target;
    ChessMove human_move;

    if (app == 0) {
        return;
    }

    if (app->localGameOver || app->localGame.board.side_to_move != CHESS_COLOR_WHITE) {
        return;
    }

    square = chess_board_view_square_at(app->screenWidth, app->screenHeight, (float)x, (float)y);
    if (square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        app_clear_selection(app);
        return;
    }

    app->lastTappedSquare = square;
    piece = chess_board_get_piece(&app->localGame.board, square);

    if (!app->hasSelection) {
        if (piece.type != CHESS_PIECE_NONE && piece.color == CHESS_COLOR_WHITE) {
            app->selectedSquare = square;
            app->hasSelection = 1;
        }
        return;
    }

    if (square == app->selectedSquare) {
        app_clear_selection(app);
        return;
    }

    target = chess_board_get_piece(&app->localGame.board, square);
    if (target.type != CHESS_PIECE_NONE && target.color == CHESS_COLOR_WHITE) {
        app->selectedSquare = square;
        app->hasSelection = 1;
        return;
    }

    moving = chess_board_get_piece(&app->localGame.board, app->selectedSquare);
    human_move.from = app->selectedSquare;
    human_move.to = square;
    human_move.promotion = CHESS_PIECE_NONE;
    if (moving.type == CHESS_PIECE_PAWN && chess_board_rank(square) == 7) {
        human_move.promotion = CHESS_PIECE_QUEEN;
    }

    if (!chess_apply_move(&app->localGame.board, &human_move)) {
        chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        app->localGameMessage = LOCAL_GAME_MESSAGE_ILLEGAL_MOVE;
        app_clear_selection(app);
        return;
    }

    chess_sound_play(target.type == CHESS_PIECE_NONE ? CHESS_SOUND_MOVE : CHESS_SOUND_CAPTURE);
    app_clear_selection(app);
    app_update_local_game_message(app, CHESS_COLOR_WHITE);

    if (!app->localGameOver && app->localGame.board.side_to_move == CHESS_COLOR_BLACK) {
        ChessMove ai_move;
        ChessPiece ai_target;

        app->localGameMessage = LOCAL_GAME_MESSAGE_AI_THINKING;
        if (chess_ai_choose_move(&app->localGame.board, CHESS_AI_NORMAL, &ai_move)) {
            ai_target = chess_board_get_piece(&app->localGame.board, ai_move.to);
            chess_apply_move(&app->localGame.board, &ai_move);
            chess_sound_play(ai_target.type == CHESS_PIECE_NONE ? CHESS_SOUND_MOVE : CHESS_SOUND_CAPTURE);
            app_update_local_game_message(app, CHESS_COLOR_BLACK);
        } else {
            app_update_local_game_message(app, CHESS_COLOR_WHITE);
        }
    }
}

void app_update(AppState* app, const InputState* input, float dt) {
    (void)dt;

    if (app == 0 || input == 0) {
        return;
    }

    if (input->backPressed) {
        app_request_back(app);
        return;
    }

    app_handle_invite_text(app, input);

    if (!input->tapReleased) {
        return;
    }

    if (app->currentScreen == APP_SCREEN_HOME) {
        screen_home_handle_tap(app, (int)input->tapX, (int)input->tapY);
        return;
    }

    if (app->currentScreen == APP_SCREEN_LOCAL_GAME) {
        if (screen_local_game_handle_tap(app, (int)input->tapX, (int)input->tapY)) {
            return;
        }
        app_handle_local_game_tap(app, (int)input->tapX, (int)input->tapY);
        return;
    }

    if (app->currentScreen == APP_SCREEN_GAME) {
        screen_game_handle_tap(app, (int)input->tapX, (int)input->tapY);
        return;
    }

    if (app->currentScreen == APP_SCREEN_CREATE_GAME) {
        screen_create_game_handle_tap(app, (int)input->tapX, (int)input->tapY);
    } else if (app->currentScreen == APP_SCREEN_JOIN_GAME) {
        screen_join_game_handle_tap(app, (int)input->tapX, (int)input->tapY);
    } else if (app->currentScreen == APP_SCREEN_MY_GAMES) {
        screen_my_games_handle_tap(app, (int)input->tapX, (int)input->tapY);
    } else if (app->currentScreen == APP_SCREEN_SETTINGS) {
        screen_settings_handle_tap(app, (int)input->tapX, (int)input->tapY);
    }
}
