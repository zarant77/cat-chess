#include "app.h"

#include "../audio/chess_sound.h"
#include "../storage/local_storage.h"
#include "../ui/chess_board_view.h"
#include "../ui/screens/screen_create_game.h"
#include "../ui/screens/screen_home.h"
#include "../ui/screens/screen_join_game.h"
#include "../ui/screens/screen_my_games.h"
#include "../ui/screens/screen_settings.h"

static void app_clear_selection(AppState* app) {
    app->selectedSquare = -1;
    app->hasSelection = 0;
}

void app_init(AppState* app) {
    char device_secret[96];

    if (app == 0) {
        return;
    }

    app->currentScreen = APP_SCREEN_HOME;
    chess_game_init(&app->localGame);
    app->selectedSquare = -1;
    app->lastTappedSquare = -1;
    app->hasSelection = 0;
    app->deviceSecretStatus = local_storage_get_device_secret(
            device_secret,
            (int)sizeof(device_secret)
    ) ? DEVICE_SECRET_STATUS_AVAILABLE : DEVICE_SECRET_STATUS_MISSING;
    app->networkStatus = NETWORK_STATUS_PLACEHOLDER;
    app->screenWidth = 0;
    app->screenHeight = 0;
    app->fps = 0;
    app->averageFrameMs = 0;
    app->exitRequested = 0;
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
    if (screen == APP_SCREEN_LOCAL_GAME) {
        app->lastTappedSquare = -1;
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

static void app_handle_local_game_tap(AppState* app, int x, int y) {
    int square;
    ChessPiece piece;
    ChessPiece target;

    if (app == 0) {
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
        if (piece.type != CHESS_PIECE_NONE && piece.color == app->localGame.board.sideToMove) {
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
    if (!chess_board_move_piece(&app->localGame.board, app->selectedSquare, square)) {
        chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        app_clear_selection(app);
        return;
    }

    chess_sound_play(target.type == CHESS_PIECE_NONE ? CHESS_SOUND_MOVE : CHESS_SOUND_CAPTURE);
    app_clear_selection(app);
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

    if (!input->tapReleased) {
        return;
    }

    if (app->currentScreen == APP_SCREEN_HOME) {
        screen_home_handle_tap(app, (int)input->tapX, (int)input->tapY);
        return;
    }

    if (app->currentScreen == APP_SCREEN_LOCAL_GAME) {
        app_handle_local_game_tap(app, (int)input->tapX, (int)input->tapY);
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
