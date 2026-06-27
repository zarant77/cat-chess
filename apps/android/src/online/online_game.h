#ifndef CAT_CHESS_ONLINE_GAME_H
#define CAT_CHESS_ONLINE_GAME_H

#include "../chess/chess_game.h"
#include "../chess/chess_move.h"
#include "cat_chess_models.h"

#define CHESS_MOVE_ANIMATION_MS 180.0f

typedef struct {
    int active;
    ChessMove move;
    ChessPiece piece;
    float elapsed_ms;
    float duration_ms;
} ChessMoveAnimation;

typedef enum {
    ONLINE_GAME_STATE_OFFLINE = 0,
    ONLINE_GAME_STATE_READY,
    ONLINE_GAME_STATE_CREATING_GAME,
    ONLINE_GAME_STATE_WAITING_FOR_OPPONENT,
    ONLINE_GAME_STATE_JOINING_GAME,
    ONLINE_GAME_STATE_ACTIVE,
    ONLINE_GAME_STATE_SUBMITTING_MOVE,
    ONLINE_GAME_STATE_POLLING,
    ONLINE_GAME_STATE_FINISHED,
    ONLINE_GAME_STATE_ERROR
} OnlineGameState;

typedef struct {
    CatChessGameDto game;
    CatChessMoveListDto moves;
    ChessGame local_game;
    ChessMoveAnimation animation;
    OnlineGameState state;
    long synced_updated_at;
    float poll_elapsed_ms;
    int is_request_in_flight;
} OnlineGame;

void online_game_init(OnlineGame* game);
int online_game_sync_board_from_game(OnlineGame* game);
int online_game_is_waiting_or_active(const OnlineGame* game);
int online_game_is_players_turn(const OnlineGame* game);
void online_game_start_move_animation(OnlineGame* game, const char* uci, ChessPiece piece);
void online_game_update_animation(OnlineGame* game, float dt);
void online_game_finish_animation(OnlineGame* game);

#endif
