#ifndef CAT_CHESS_ONLINE_GAME_H
#define CAT_CHESS_ONLINE_GAME_H

#define CAT_CHESS_INVITE_CODE_MAX 32

typedef enum {
    ONLINE_GAME_PLACEHOLDER_IDLE = 0,
    ONLINE_GAME_PLACEHOLDER_LOADING,
    ONLINE_GAME_PLACEHOLDER_ERROR
} OnlineGamePlaceholderState;

typedef struct {
    int gameId;
    char inviteCode[CAT_CHESS_INVITE_CODE_MAX];
    OnlineGamePlaceholderState state;
} OnlineGame;

void online_game_init(OnlineGame* game);

#endif
