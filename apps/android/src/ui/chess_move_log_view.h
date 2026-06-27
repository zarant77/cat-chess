#ifndef CAT_CHESS_MOVE_LOG_VIEW_H
#define CAT_CHESS_MOVE_LOG_VIEW_H

#include "../app/app_state.h"
#include "../chess/chess_move_log.h"
#include "../renderer/renderer.h"
#include "ui_controls.h"

void chess_move_log_view_render(
        Framebuffer* framebuffer,
        const AppState* app,
        const ChessMoveLogEntry* entries,
        int entry_count,
        UiRect rect
);

#endif
