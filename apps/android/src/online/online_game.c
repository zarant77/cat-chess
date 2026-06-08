#include "online_game.h"

void online_game_init(OnlineGame* game) {
    if (game == 0) {
        return;
    }

    cat_chess_game_dto_init(&game->game);
    cat_chess_move_list_dto_init(&game->moves);
    game->state = ONLINE_GAME_PLACEHOLDER_IDLE;
}
