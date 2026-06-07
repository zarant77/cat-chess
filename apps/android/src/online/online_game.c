#include "online_game.h"

void online_game_init(OnlineGame* game) {
    if (game == 0) {
        return;
    }

    cat_chess_game_dto_init(&game->game);
    game->move_count = 0;
    for (int index = 0; index < CAT_CHESS_MOVE_LIST_MAX; ++index) {
        cat_chess_move_dto_init(game->moves + index);
    }
    game->state = ONLINE_GAME_PLACEHOLDER_IDLE;
}
