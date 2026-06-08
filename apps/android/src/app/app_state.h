#ifndef CAT_CHESS_APP_STATE_H
#define CAT_CHESS_APP_STATE_H

#include "../chess/chess_game.h"
#include "../online/cat_chess_api.h"
#include "../online/online_game.h"
#include "../settings/game_settings.h"

typedef enum {
    APP_SCREEN_HOME = 0,
    APP_SCREEN_LOCAL_GAME,
    APP_SCREEN_CREATE_GAME,
    APP_SCREEN_JOIN_GAME,
    APP_SCREEN_MY_GAMES,
    APP_SCREEN_GAME,
    APP_SCREEN_SETTINGS
} AppScreen;

typedef enum {
    DEVICE_SECRET_STATUS_UNKNOWN = 0,
    DEVICE_SECRET_STATUS_MISSING,
    DEVICE_SECRET_STATUS_AVAILABLE
} DeviceSecretStatus;

typedef enum {
    NETWORK_STATUS_OFFLINE = 0,
    NETWORK_STATUS_READY,
    NETWORK_STATUS_NOT_IMPLEMENTED,
    NETWORK_STATUS_ERROR
} NetworkStatus;

typedef struct {
    AppScreen currentScreen;
    ChessGame localGame;
    GameSettings settings;
    CatChessApiClient apiClient;
    OnlineGame onlineGame;
    CatChessGameListDto games;
    CatChessApiStatus lastApiStatus;
    char inviteCode[CAT_CHESS_INVITE_CODE_MAX];
    int inviteInputFocused;
    int selectedSquare;
    int lastTappedSquare;
    int hasSelection;
    DeviceSecretStatus deviceSecretStatus;
    NetworkStatus networkStatus;
    int screenWidth;
    int screenHeight;
    int fps;
    int averageFrameMs;
    int exitRequested;
    int softKeyboardRequested;
} AppState;

#endif
