#include "app.h"

#include <string.h>

#include <android/log.h>

#include "../audio/audio.h"
#include "../audio/chess_sound.h"
#include "../cat_chess_network_config.h"
#include "../chess/chess_ai.h"
#include "../chess/chess_rules.h"
#include "../config.h"
#include "../online/cat_chess_models.h"
#include "../storage/local_storage.h"
#include "../ui/chess_board_view.h"
#include "../ui/screens/screen_create_game.h"
#include "../ui/screens/screen_game.h"
#include "../ui/screens/screen_home.h"
#include "../ui/screens/screen_join_game.h"
#include "../ui/screens/screen_my_games.h"
#include "../ui/screens/screen_online.h"
#include "../ui/screens/screen_settings.h"
#include "../ui/screens/screen_local_game.h"

#ifndef CAT_CHESS_DEBUG_NETWORK
#define CAT_CHESS_DEBUG_NETWORK 0
#endif

#if CAT_CHESS_DEBUG_NETWORK
#define ONLINE_LOG(...) __android_log_print(ANDROID_LOG_INFO, CAT_CHESS_LOG_TAG, __VA_ARGS__)
#else
#define ONLINE_LOG(...) ((void)0)
#endif

static void app_clear_selection(AppState* app) {
    app->selectedSquare = -1;
    app->hasSelection = 0;
    app->moveHints.count = 0;
}

static int app_is_promotion_move(const ChessBoard* board, const ChessMove* move) {
    ChessPiece moving;

    if (board == 0 || move == 0) {
        return 0;
    }

    moving = chess_board_get_piece(board, move->from);
    return moving.type == CHESS_PIECE_PAWN
            && (chess_board_rank(move->to) == 0 || chess_board_rank(move->to) == 7);
}

static char app_promotion_char(ChessPieceType promotion) {
    if (promotion == CHESS_PIECE_QUEEN) {
        return 'q';
    }
    if (promotion == CHESS_PIECE_ROOK) {
        return 'r';
    }
    if (promotion == CHESS_PIECE_BISHOP) {
        return 'b';
    }
    if (promotion == CHESS_PIECE_KNIGHT) {
        return 'n';
    }
    return '\0';
}

static void app_update_move_hints(AppState* app, const ChessGame* game) {
    if (app == 0) {
        return;
    }

    app->moveHints.count = 0;
    if (game == 0
            || !app->settings.show_move_hints
            || !app->hasSelection
            || app->selectedSquare < 0
            || app->selectedSquare >= CHESS_BOARD_SQUARE_COUNT) {
        return;
    }

    chess_game_get_legal_moves_for_square(game, app->selectedSquare, &app->moveHints);
}

static void app_select_square(AppState* app, const ChessGame* game, int square) {
    if (app == 0) {
        return;
    }

    app->selectedSquare = square;
    app->hasSelection = 1;
    app_update_move_hints(app, game);
}

static void app_clear_move_animation(ChessMoveAnimation* animation) {
    if (animation == 0) {
        return;
    }

    animation->active = 0;
    animation->move.from = -1;
    animation->move.to = -1;
    animation->move.promotion = CHESS_PIECE_NONE;
    animation->piece = chess_piece_empty();
    animation->elapsed_ms = 0.0f;
    animation->duration_ms = CHESS_MOVE_ANIMATION_MS;
}

static void app_clear_local_history(AppState* app) {
    if (app == 0) {
        return;
    }

    app->localHistoryCount = 0;
}

static void app_clear_local_move_log(AppState* app) {
    if (app == 0) {
        return;
    }

    app->localMoveLogCount = 0;
}

static void app_append_local_move_log(AppState* app, const ChessBoard* before, const ChessMove* move) {
    ChessBoard after;

    if (app == 0 || before == 0 || move == 0 || app->localMoveLogCount >= CHESS_MOVE_LOG_MAX) {
        return;
    }

    after = *before;
    if (!chess_apply_move(&after, move)) {
        return;
    }

    chess_move_log_entry_from_move(
            before,
            move,
            &after,
            &app->localMoveLog[app->localMoveLogCount]
    );
    app->localMoveLogCount += 1;
}

static void app_push_local_history(AppState* app) {
    LocalGameSnapshot* snapshot;

    if (app == 0) {
        return;
    }

    if (app->localHistoryCount >= LOCAL_GAME_HISTORY_MAX) {
        memmove(
                &app->localHistory[0],
                &app->localHistory[1],
                sizeof(app->localHistory[0]) * (LOCAL_GAME_HISTORY_MAX - 1)
        );
        app->localHistoryCount = LOCAL_GAME_HISTORY_MAX - 1;
    }

    snapshot = &app->localHistory[app->localHistoryCount];
    snapshot->board = app->localGame.board;
    snapshot->message = app->localGameMessage;
    snapshot->game_over = app->localGameOver;
    snapshot->move_log_count = app->localMoveLogCount;
    app->localHistoryCount += 1;
}

static void app_start_move_animation(ChessMoveAnimation* animation, ChessMove move, ChessPiece piece) {
    if (animation == 0 || piece.type == CHESS_PIECE_NONE) {
        return;
    }

    animation->active = 1;
    animation->move = move;
    animation->piece = piece;
    animation->elapsed_ms = 0.0f;
    animation->duration_ms = CHESS_MOVE_ANIMATION_MS;
}

static void app_update_move_animation(ChessMoveAnimation* animation, float dt) {
    if (animation == 0 || !animation->active) {
        return;
    }

    animation->elapsed_ms += dt * 1000.0f;
    if (animation->elapsed_ms >= animation->duration_ms) {
        animation->active = 0;
        animation->elapsed_ms = 0.0f;
    }
}

static void app_start_local_game(AppState* app) {
    if (app == 0) {
        return;
    }

    chess_game_init(&app->localGame);
    app_clear_move_animation(&app->localAnimation);
    app->localGameMessage = LOCAL_GAME_MESSAGE_YOUR_MOVE;
    app->localGameOver = 0;
    app->localAiMovePending = 0;
    app->localAiGeneration = 0;
    app->localPendingAiGeneration = 0;
    app->promotionPickerVisible = 0;
    app->promotionPickerOnline = 0;
    app_clear_local_history(app);
    app_clear_local_move_log(app);
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

int app_can_undo_local_game(const AppState* app) {
    return app != 0
            && app->currentScreen == APP_SCREEN_LOCAL_GAME
            && app->localHistoryCount > 0;
}

void app_undo_local_game(AppState* app) {
    LocalGameSnapshot snapshot;

    if (!app_can_undo_local_game(app)) {
        return;
    }

    snapshot = app->localHistory[app->localHistoryCount - 1];
    app->localHistoryCount -= 1;
    app->localGame.board = snapshot.board;
    app->localGameMessage = snapshot.message;
    app->localGameOver = snapshot.game_over;
    app->localMoveLogCount = snapshot.move_log_count;
    app->localAiGeneration += 1;
    app->localPendingAiGeneration = 0;
    app->localAiMovePending = 0;
    app->promotionPickerVisible = 0;
    app_clear_move_animation(&app->localAnimation);
    app->lastTappedSquare = -1;
    app_clear_selection(app);
    app_update_local_game_message(app, CHESS_COLOR_BLACK);
    chess_sound_play(CHESS_SOUND_MENU_BACK);
}

static void app_play_local_move_sound(const AppState* app, ChessPiece target) {
    ChessGameStatus status;

    if (app == 0) {
        return;
    }

    status = chess_get_game_status(&app->localGame.board);
    if (status == CHESS_GAME_CHECKMATE) {
        chess_sound_play(CHESS_SOUND_CHECKMATE);
        return;
    }
    if (chess_is_in_check(&app->localGame.board, app->localGame.board.side_to_move)) {
        chess_sound_play(CHESS_SOUND_CHECK);
        return;
    }

    chess_sound_play(target.type == CHESS_PIECE_NONE ? CHESS_SOUND_MOVE : CHESS_SOUND_CAPTURE);
}

static void app_play_online_move_sound(const AppState* app, ChessPiece target) {
    if (app == 0) {
        return;
    }
    if (app->onlineGame.game.status == CAT_CHESS_GAME_STATUS_FINISHED) {
        chess_sound_play(CHESS_SOUND_CHECKMATE);
        return;
    }
    if (chess_is_in_check(&app->onlineGame.local_game.board, app->onlineGame.local_game.board.side_to_move)) {
        chess_sound_play(CHESS_SOUND_CHECK);
        return;
    }

    chess_sound_play(target.type == CHESS_PIECE_NONE ? CHESS_SOUND_MOVE : CHESS_SOUND_CAPTURE);
}

static ChessColor app_chess_color_from_player(CatChessPlayerColor color) {
    return color == CAT_CHESS_PLAYER_COLOR_BLACK ? CHESS_COLOR_BLACK : CHESS_COLOR_WHITE;
}

static void app_start_online_animation_from_uci(AppState* app, const char* uci) {
    ChessMove move;
    ChessPiece piece;

    if (app == 0 || uci == 0 || uci[0] == '\0') {
        return;
    }
    if (app->onlineGame.animation.active) {
        online_game_finish_animation(&app->onlineGame);
    }
    if (!chess_move_parse_uci(uci, &move)) {
        return;
    }

    piece = chess_board_get_piece(&app->onlineGame.local_game.board, move.from);
    online_game_start_move_animation(&app->onlineGame, uci, piece);
}

static void app_update_online_state_from_game(AppState* app) {
    if (app == 0) {
        return;
    }

    if (!online_game_sync_board_from_game(&app->onlineGame)) {
        app->onlineGame.state = ONLINE_GAME_STATE_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
        return;
    }

    app->networkStatus = NETWORK_STATUS_READY;
}

void app_save_settings(AppState* app) {
    if (app == 0) {
        return;
    }

    audio_set_sounds_enabled(app->settings.sounds_enabled);
    audio_set_sfx_volume(app->settings.sfx_volume);
    audio_set_music_enabled(app->settings.music_enabled);
    if (app->settings.music_enabled) {
        audio_play_music("chess_theme");
    }
    local_storage_set_settings(&app->settings);
}

void app_init(AppState* app) {
    char device_secret[CAT_CHESS_DEVICE_SECRET_MAX];

    if (app == 0) {
        return;
    }

    app->currentScreen = APP_SCREEN_HOME;
    chess_game_init(&app->localGame);
    app_clear_move_animation(&app->localAnimation);
    app->localGameMessage = LOCAL_GAME_MESSAGE_YOUR_MOVE;
    app->localGameOver = 0;
    app->localAiMovePending = 0;
    app->localAiGeneration = 0;
    app->localPendingAiGeneration = 0;
    app->localHistoryCount = 0;
    app->localMoveLogCount = 0;
    app->exitConfirmVisible = 0;
    app->promotionPickerVisible = 0;
    app->promotionPickerOnline = 0;
    app->pendingPromotionMove.from = -1;
    app->pendingPromotionMove.to = -1;
    app->pendingPromotionMove.promotion = CHESS_PIECE_NONE;
    app->settingsReturnScreen = APP_SCREEN_HOME;
    if (!local_storage_get_settings(&app->settings)) {
        game_settings_init(&app->settings);
        local_storage_set_settings(&app->settings);
    }
    audio_set_sounds_enabled(app->settings.sounds_enabled);
    audio_set_sfx_volume(app->settings.sfx_volume);
    audio_set_music_enabled(app->settings.music_enabled);
    if (app->settings.music_enabled) {
        audio_play_music("chess_theme");
    }
    cat_chess_api_init(&app->apiClient, CAT_CHESS_SERVER_URL);
    online_game_init(&app->onlineGame);
    cat_chess_game_list_dto_init(&app->games);
    app->lastApiStatus.result = CAT_CHESS_API_OK;
    app->lastApiStatus.http_status = 0;
    app->lastApiStatus.error_code[0] = '\0';
    app->inviteCode[0] = '\0';
    app->inviteInputFocused = 0;
    app->myGamesPage = 0;
    app->selectedSquare = -1;
    app->lastTappedSquare = -1;
    app->hasSelection = 0;
    app->moveHints.count = 0;
    if (local_storage_get_device_secret(device_secret, (int)sizeof(device_secret))) {
        cat_chess_api_set_device_secret(&app->apiClient, device_secret);
        app->deviceSecretStatus = DEVICE_SECRET_STATUS_AVAILABLE;
        app->networkStatus = NETWORK_STATUS_READY;
        app->onlineGame.state = ONLINE_GAME_STATE_READY;
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
            app->onlineGame.state = ONLINE_GAME_STATE_READY;
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
    AppScreen previous_screen;

    if (app == 0) {
        return;
    }
    previous_screen = app->currentScreen;
    if (screen == APP_SCREEN_SETTINGS && app->currentScreen != APP_SCREEN_SETTINGS) {
        app->settingsReturnScreen = app->currentScreen;
    }
    if (app->currentScreen != screen && screen != APP_SCREEN_GAME) {
        chess_sound_play(screen == APP_SCREEN_HOME ? CHESS_SOUND_MENU_BACK : CHESS_SOUND_MENU_SELECT);
    }

    if (screen != APP_SCREEN_GAME) {
        online_game_finish_animation(&app->onlineGame);
    }
    if (screen != APP_SCREEN_LOCAL_GAME
            && !(screen == APP_SCREEN_SETTINGS && previous_screen == APP_SCREEN_LOCAL_GAME)) {
        app_clear_move_animation(&app->localAnimation);
        app->localAiMovePending = 0;
        app->localAiGeneration += 1;
        app->localPendingAiGeneration = 0;
    }
    app->currentScreen = screen;
    app->exitConfirmVisible = 0;
    if (screen != APP_SCREEN_LOCAL_GAME && screen != APP_SCREEN_GAME) {
        app->promotionPickerVisible = 0;
    }
    app_clear_selection(app);
    app->inviteInputFocused = 0;
    if (screen == APP_SCREEN_LOCAL_GAME && previous_screen != APP_SCREEN_SETTINGS) {
        app_start_local_game(app);
    } else if (screen == APP_SCREEN_MY_GAMES) {
        app_load_my_games(app);
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

    app->onlineGame.state = ONLINE_GAME_STATE_CREATING_GAME;
    app->onlineGame.is_request_in_flight = 1;
    app->lastApiStatus = cat_chess_api_create_game(&app->apiClient, &app->onlineGame.game);
    app->onlineGame.is_request_in_flight = 0;
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        ONLINE_LOG(
                "created game id=%d invite=%s color=%s side=%s fen=%s",
                app->onlineGame.game.id,
                app->onlineGame.game.invite_code,
                cat_chess_player_color_string(app->onlineGame.game.your_color),
                cat_chess_player_color_string(app->onlineGame.game.side_to_move),
                app->onlineGame.game.board_fen
        );
        app_update_online_state_from_game(app);
        app_clear_selection(app);
        chess_sound_play(CHESS_SOUND_GAME_CREATED);
        app_navigate(app, APP_SCREEN_GAME);
    } else {
        app->onlineGame.state = ONLINE_GAME_STATE_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
    }
}

void app_join_online_game(AppState* app) {
    if (app == 0 || app->inviteCode[0] == '\0' || !app_ensure_device_secret(app)) {
        return;
    }

    app->onlineGame.state = ONLINE_GAME_STATE_JOINING_GAME;
    app->onlineGame.is_request_in_flight = 1;
    app->lastApiStatus = cat_chess_api_join_game(&app->apiClient, app->inviteCode, &app->onlineGame.game);
    app->onlineGame.is_request_in_flight = 0;
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        ONLINE_LOG(
                "joined game id=%d color=%s side=%s fen=%s",
                app->onlineGame.game.id,
                cat_chess_player_color_string(app->onlineGame.game.your_color),
                cat_chess_player_color_string(app->onlineGame.game.side_to_move),
                app->onlineGame.game.board_fen
        );
        app_update_online_state_from_game(app);
        app_clear_selection(app);
        chess_sound_play(CHESS_SOUND_GAME_JOINED);
        app_navigate(app, APP_SCREEN_GAME);
    } else {
        app->onlineGame.state = ONLINE_GAME_STATE_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
    }
}

void app_load_my_games(AppState* app) {
    if (app == 0 || !app_ensure_device_secret(app)) {
        return;
    }

    app->onlineGame.is_request_in_flight = 1;
    app->lastApiStatus = cat_chess_api_list_games(&app->apiClient, &app->games);
    app->onlineGame.is_request_in_flight = 0;
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        app->myGamesPage = 0;
        app->networkStatus = NETWORK_STATUS_READY;
    } else {
        app->networkStatus = NETWORK_STATUS_ERROR;
    }
}

void app_open_listed_online_game(AppState* app, int index) {
    int game_id;

    if (app == 0 || index < 0 || index >= app->games.count || !app_ensure_device_secret(app)) {
        return;
    }

    game_id = app->games.items[index].id;
    app->onlineGame.is_request_in_flight = 1;
    app->lastApiStatus = cat_chess_api_get_game(&app->apiClient, game_id, &app->onlineGame.game);
    app->onlineGame.is_request_in_flight = 0;
    if (app->lastApiStatus.result != CAT_CHESS_API_OK) {
        app->onlineGame.state = ONLINE_GAME_STATE_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
        return;
    }

    if (!online_game_sync_board_from_game(&app->onlineGame)) {
        app->onlineGame.state = ONLINE_GAME_STATE_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
        return;
    }

    app->onlineGame.is_request_in_flight = 1;
    app->lastApiStatus = cat_chess_api_get_moves(&app->apiClient, game_id, &app->onlineGame.moves);
    app->onlineGame.is_request_in_flight = 0;
    if (app->lastApiStatus.result != CAT_CHESS_API_OK) {
        app->onlineGame.state = ONLINE_GAME_STATE_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
        return;
    }

    app->networkStatus = NETWORK_STATUS_READY;
    app_clear_selection(app);
    app_navigate(app, APP_SCREEN_GAME);
}

void app_refresh_online_game(AppState* app) {
    long previous_updated_at;
    char previous_last_move[CAT_CHESS_UCI_MAX];

    if (app == 0
            || app->onlineGame.game.id <= 0
            || app->onlineGame.is_request_in_flight
            || !app_ensure_device_secret(app)) {
        return;
    }

    previous_updated_at = app->onlineGame.game.updated_at;
    strncpy(previous_last_move, app->onlineGame.game.last_move, sizeof(previous_last_move) - 1u);
    previous_last_move[sizeof(previous_last_move) - 1u] = '\0';
    app->onlineGame.state = ONLINE_GAME_STATE_POLLING;
    app->onlineGame.is_request_in_flight = 1;
    app->lastApiStatus = cat_chess_api_get_game(&app->apiClient, app->onlineGame.game.id, &app->onlineGame.game);
    app->onlineGame.is_request_in_flight = 0;
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        if (app->onlineGame.game.updated_at != previous_updated_at) {
            if (app->onlineGame.game.last_move[0] != '\0'
                    && strcmp(app->onlineGame.game.last_move, previous_last_move) != 0) {
                app_start_online_animation_from_uci(app, app->onlineGame.game.last_move);
            }
            ONLINE_LOG(
                    "polled game id=%d updated_at=%ld color=%s side=%s fen=%s",
                    app->onlineGame.game.id,
                    app->onlineGame.game.updated_at,
                    cat_chess_player_color_string(app->onlineGame.game.your_color),
                    cat_chess_player_color_string(app->onlineGame.game.side_to_move),
                    app->onlineGame.game.board_fen
            );
        }
        app_update_online_state_from_game(app);
        if (app->onlineGame.game.updated_at != previous_updated_at) {
            app_clear_selection(app);
        }
        if (app->onlineGame.game.updated_at != previous_updated_at
                || app->onlineGame.moves.count == 0) {
            app->onlineGame.is_request_in_flight = 1;
            app->lastApiStatus = cat_chess_api_get_moves(&app->apiClient, app->onlineGame.game.id, &app->onlineGame.moves);
            app->onlineGame.is_request_in_flight = 0;
        }
    } else {
        app->onlineGame.state = ONLINE_GAME_STATE_ERROR;
        app->networkStatus = NETWORK_STATUS_ERROR;
    }
}

static void app_update_online_poll(AppState* app, float dt) {
    if (app != 0) {
        online_game_update_animation(&app->onlineGame, dt);
    }

    if (app == 0 || app->currentScreen != APP_SCREEN_GAME || !online_game_is_waiting_or_active(&app->onlineGame)) {
        return;
    }

    app->onlineGame.poll_elapsed_ms += dt * 1000.0f;
    if (app->onlineGame.poll_elapsed_ms < (float)CAT_CHESS_POLL_INTERVAL_MS) {
        return;
    }

    app->onlineGame.poll_elapsed_ms = 0.0f;
    app_refresh_online_game(app);
}

static void app_update_local_animation(AppState* app, float dt) {
    int was_active;
    int ai_generation;
    ChessMove ai_move;
    ChessPiece moving;
    ChessPiece target;

    if (app == 0 || app->currentScreen != APP_SCREEN_LOCAL_GAME) {
        return;
    }

    was_active = app->localAnimation.active;
    app_update_move_animation(&app->localAnimation, dt);
    if (was_active && app->localAnimation.active) {
        return;
    }
    if (app->localAnimation.active
            || !app->localAiMovePending
            || app->localGameOver
            || app->localGame.board.side_to_move != CHESS_COLOR_BLACK) {
        return;
    }

    app->localAiMovePending = 0;
    ai_generation = app->localPendingAiGeneration;
    app->localGameMessage = LOCAL_GAME_MESSAGE_AI_THINKING;
    if (!chess_ai_choose_move(
            &app->localGame.board,
            game_settings_normalize_ai_difficulty(app->settings.ai_difficulty),
            &ai_move
    )) {
        app_update_local_game_message(app, CHESS_COLOR_WHITE);
        return;
    }
    if (ai_generation != app->localAiGeneration
            || app->localGame.board.side_to_move != CHESS_COLOR_BLACK
            || app->localGameOver) {
        return;
    }

    moving = chess_board_get_piece(&app->localGame.board, ai_move.from);
    target = chess_board_get_piece(&app->localGame.board, ai_move.to);
    app_append_local_move_log(app, &app->localGame.board, &ai_move);
    chess_apply_move(&app->localGame.board, &ai_move);
    app_start_move_animation(&app->localAnimation, ai_move, moving);
    app_play_local_move_sound(app, target);
    app_update_local_game_message(app, CHESS_COLOR_BLACK);
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

    if (app->promotionPickerVisible) {
        return;
    }

    if (app->exitConfirmVisible) {
        app_cancel_exit_confirmation(app);
        return;
    }

    if (app->currentScreen == APP_SCREEN_HOME) {
        app->exitRequested = 1;
        return;
    }

    if (app->currentScreen == APP_SCREEN_LOCAL_GAME || app->currentScreen == APP_SCREEN_GAME) {
        app_request_exit_confirmation(app);
        return;
    }

    if (app->currentScreen == APP_SCREEN_SETTINGS) {
        app_navigate(app, app->settingsReturnScreen);
        return;
    }

    app_navigate(app, APP_SCREEN_HOME);
}

void app_open_settings(AppState* app) {
    if (app == 0) {
        return;
    }

    app->exitConfirmVisible = 0;
    app_navigate(app, APP_SCREEN_SETTINGS);
}

void app_request_exit_confirmation(AppState* app) {
    if (app == 0) {
        return;
    }

    app->exitConfirmVisible = 1;
    app_clear_selection(app);
    chess_sound_play(CHESS_SOUND_MENU_SELECT);
}

void app_cancel_exit_confirmation(AppState* app) {
    if (app == 0) {
        return;
    }

    app->exitConfirmVisible = 0;
    chess_sound_play(CHESS_SOUND_MENU_BACK);
}

void app_confirm_exit_to_home(AppState* app) {
    if (app == 0) {
        return;
    }

    app->exitConfirmVisible = 0;
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

static void app_complete_local_human_move(AppState* app, ChessMove human_move) {
    ChessPiece moving;
    ChessPiece target;

    if (app == 0 || !chess_is_move_legal(&app->localGame.board, &human_move)) {
        if (app != 0) {
            chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
            app->localGameMessage = LOCAL_GAME_MESSAGE_ILLEGAL_MOVE;
            app_clear_selection(app);
        }
        return;
    }

    moving = chess_board_get_piece(&app->localGame.board, human_move.from);
    target = chess_move_log_captured_piece(&app->localGame.board, &human_move);
    app_push_local_history(app);
    app_append_local_move_log(app, &app->localGame.board, &human_move);
    chess_apply_move(&app->localGame.board, &human_move);
    app_start_move_animation(&app->localAnimation, human_move, moving);
    app_clear_selection(app);
    app_play_local_move_sound(app, target);
    app_update_local_game_message(app, CHESS_COLOR_WHITE);

    if (!app->localGameOver && app->localGame.board.side_to_move == CHESS_COLOR_BLACK) {
        app->localAiGeneration += 1;
        app->localPendingAiGeneration = app->localAiGeneration;
        app->localAiMovePending = 1;
        app->localGameMessage = LOCAL_GAME_MESSAGE_AI_THINKING;
    }
}

static void app_submit_online_move(AppState* app, ChessMove move) {
    ChessPiece target;
    char uci[CAT_CHESS_UCI_MAX];
    CatChessGameDto updated_game;

    if (app == 0) {
        return;
    }

    if (!chess_is_move_legal(&app->onlineGame.local_game.board, &move)) {
        chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        app_clear_selection(app);
        return;
    }

    target = chess_move_log_captured_piece(&app->onlineGame.local_game.board, &move);
    cat_chess_api_build_uci(
            move.from,
            move.to,
            app_promotion_char(move.promotion),
            uci,
            (int)sizeof(uci)
    );
    if (!cat_chess_api_is_uci_move(uci)) {
        chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        app_clear_selection(app);
        return;
    }

    app->onlineGame.state = ONLINE_GAME_STATE_SUBMITTING_MOVE;
    app->onlineGame.is_request_in_flight = 1;
    app->lastApiStatus = cat_chess_api_make_move(&app->apiClient, app->onlineGame.game.id, uci, &updated_game);
    app->onlineGame.is_request_in_flight = 0;
    if (app->lastApiStatus.result == CAT_CHESS_API_OK) {
        app_start_online_animation_from_uci(app, updated_game.last_move[0] != '\0' ? updated_game.last_move : uci);
        app->onlineGame.game = updated_game;
        ONLINE_LOG(
                "submitted move game id=%d uci=%s color=%s side=%s fen=%s",
                app->onlineGame.game.id,
                uci,
                cat_chess_player_color_string(app->onlineGame.game.your_color),
                cat_chess_player_color_string(app->onlineGame.game.side_to_move),
                app->onlineGame.game.board_fen
        );
        app_update_online_state_from_game(app);
        app->onlineGame.is_request_in_flight = 1;
        app->lastApiStatus = cat_chess_api_get_moves(&app->apiClient, app->onlineGame.game.id, &app->onlineGame.moves);
        app->onlineGame.is_request_in_flight = 0;
        app_play_online_move_sound(app, target);
    } else {
        chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        app_refresh_online_game(app);
    }

    app_clear_selection(app);
}

static void app_open_promotion_picker(AppState* app, ChessMove move, int online) {
    if (app == 0) {
        return;
    }

    app->promotionPickerVisible = 1;
    app->promotionPickerOnline = online;
    app->pendingPromotionMove = move;
    app_clear_selection(app);
}

void app_choose_promotion(AppState* app, ChessPieceType promotion) {
    ChessMove move;

    if (app == 0 || !app->promotionPickerVisible) {
        return;
    }
    if (promotion != CHESS_PIECE_QUEEN
            && promotion != CHESS_PIECE_ROOK
            && promotion != CHESS_PIECE_BISHOP
            && promotion != CHESS_PIECE_KNIGHT) {
        return;
    }

    move = app->pendingPromotionMove;
    move.promotion = promotion;
    app->promotionPickerVisible = 0;
    if (app->promotionPickerOnline) {
        app_submit_online_move(app, move);
    } else {
        app_complete_local_human_move(app, move);
    }
}

static void app_handle_local_game_tap(AppState* app, int x, int y) {
    int square;
    ChessPiece piece;
    ChessPiece target;
    ChessMove human_move;

    if (app == 0) {
        return;
    }

    if (app->localGameOver
            || app->localAnimation.active
            || app->localAiMovePending
            || app->localGame.board.side_to_move != CHESS_COLOR_WHITE) {
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
            app_select_square(app, &app->localGame, square);
        }
        return;
    }

    if (square == app->selectedSquare) {
        app_clear_selection(app);
        return;
    }

    target = chess_board_get_piece(&app->localGame.board, square);
    if (target.type != CHESS_PIECE_NONE && target.color == CHESS_COLOR_WHITE) {
        app_select_square(app, &app->localGame, square);
        return;
    }

    human_move.from = app->selectedSquare;
    human_move.to = square;
    human_move.promotion = CHESS_PIECE_NONE;
    if (app_is_promotion_move(&app->localGame.board, &human_move)) {
        human_move.promotion = CHESS_PIECE_QUEEN;
    }

    if (!chess_is_move_legal(&app->localGame.board, &human_move)) {
        chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        app->localGameMessage = LOCAL_GAME_MESSAGE_ILLEGAL_MOVE;
        app_clear_selection(app);
        return;
    }

    if (app_is_promotion_move(&app->localGame.board, &human_move)) {
        human_move.promotion = CHESS_PIECE_NONE;
        app_open_promotion_picker(app, human_move, 0);
        return;
    }

    app_complete_local_human_move(app, human_move);
}

static void app_handle_online_game_tap(AppState* app, int x, int y) {
    int square;
    ChessPiece piece;
    ChessPiece target;
    ChessMove move;
    ChessColor player_color;

    if (app == 0 || app->onlineGame.game.id <= 0) {
        return;
    }

    if (!online_game_is_players_turn(&app->onlineGame)
            || app->onlineGame.is_request_in_flight
            || app->onlineGame.animation.active) {
        app_clear_selection(app);
        return;
    }

    player_color = app_chess_color_from_player(app->onlineGame.game.your_color);
    square = chess_board_view_square_at_for_color(
            app->screenWidth,
            app->screenHeight,
            (float)x,
            (float)y,
            player_color
    );
    if (square < 0 || square >= CHESS_BOARD_SQUARE_COUNT) {
        app_clear_selection(app);
        return;
    }

    app->lastTappedSquare = square;
    piece = chess_board_get_piece(&app->onlineGame.local_game.board, square);

    if (!app->hasSelection) {
        if (piece.type != CHESS_PIECE_NONE && piece.color == player_color) {
            app_select_square(app, &app->onlineGame.local_game, square);
        }
        return;
    }

    if (square == app->selectedSquare) {
        app_clear_selection(app);
        return;
    }

    target = chess_board_get_piece(&app->onlineGame.local_game.board, square);
    if (target.type != CHESS_PIECE_NONE && target.color == player_color) {
        app_select_square(app, &app->onlineGame.local_game, square);
        return;
    }

    move.from = app->selectedSquare;
    move.to = square;
    move.promotion = CHESS_PIECE_NONE;
    if (app_is_promotion_move(&app->onlineGame.local_game.board, &move)) {
        move.promotion = CHESS_PIECE_QUEEN;
    }
    if (!chess_is_move_legal(&app->onlineGame.local_game.board, &move)) {
        chess_sound_play(CHESS_SOUND_ILLEGAL_MOVE);
        app_clear_selection(app);
        return;
    }
    if (app_is_promotion_move(&app->onlineGame.local_game.board, &move)) {
        move.promotion = CHESS_PIECE_NONE;
        app_open_promotion_picker(app, move, 1);
        return;
    }

    app_submit_online_move(app, move);
}

void app_update(AppState* app, const InputState* input, float dt) {
    if (app == 0 || input == 0) {
        return;
    }

    if (input->backPressed) {
        app_request_back(app);
        return;
    }

    app_handle_invite_text(app, input);
    app_update_online_poll(app, dt);
    app_update_local_animation(app, dt);

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
        if (screen_game_handle_tap(app, (int)input->tapX, (int)input->tapY)) {
            return;
        }
        app_handle_online_game_tap(app, (int)input->tapX, (int)input->tapY);
        return;
    }

    if (app->currentScreen == APP_SCREEN_ONLINE) {
        screen_online_handle_tap(app, (int)input->tapX, (int)input->tapY);
    } else if (app->currentScreen == APP_SCREEN_CREATE_GAME) {
        screen_create_game_handle_tap(app, (int)input->tapX, (int)input->tapY);
    } else if (app->currentScreen == APP_SCREEN_JOIN_GAME) {
        screen_join_game_handle_tap(app, (int)input->tapX, (int)input->tapY);
    } else if (app->currentScreen == APP_SCREEN_MY_GAMES) {
        screen_my_games_handle_tap(app, (int)input->tapX, (int)input->tapY);
    } else if (app->currentScreen == APP_SCREEN_SETTINGS) {
        screen_settings_handle_tap(app, (int)input->tapX, (int)input->tapY);
    }
}
