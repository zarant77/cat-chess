#include "online_game.h"

void online_game_init(OnlineGame* game) {
    if (game == 0) {
        return;
    }

    game->gameId = 0;
    game->inviteCode[0] = '\0';
    game->state = ONLINE_GAME_PLACEHOLDER_IDLE;
}
