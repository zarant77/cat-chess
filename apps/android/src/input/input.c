#include "input.h"

#define INPUT_NO_POINTER -1

void input_init(InputState* input) {
    if (input == 0) {
        return;
    }

    input->activePointerId = INPUT_NO_POINTER;
    input->touchActive = 0;
    input->tapReleased = 0;
    input->tapSquare = -1;
    input->touchX = 0.0f;
    input->touchY = 0.0f;
}

void input_handle_touch(
        InputState* input,
        int action_type,
        int pointer_id,
        float x,
        float y,
        float screen_width
) {
    (void)screen_width;

    if (input == 0) {
        return;
    }

    if (action_type == INPUT_TOUCH_DOWN) {
        if (!input->touchActive) {
            input->activePointerId = pointer_id;
            input->touchActive = 1;
            input->touchX = x;
            input->touchY = y;
        }
        return;
    }

    if (action_type == INPUT_TOUCH_MOVE) {
        if (pointer_id == input->activePointerId) {
            input->touchX = x;
            input->touchY = y;
        }
        return;
    }

    if (action_type == INPUT_TOUCH_UP) {
        if (pointer_id == input->activePointerId) {
            input->touchActive = 0;
            input->activePointerId = INPUT_NO_POINTER;
            input->tapReleased = 1;
        }
        return;
    }

    if (action_type == INPUT_TOUCH_CANCEL) {
        input->activePointerId = INPUT_NO_POINTER;
        input->touchActive = 0;
        input->tapReleased = 0;
        input->tapSquare = -1;
    }
}

void input_end_frame(InputState* input) {
    if (input == 0) {
        return;
    }

    input->tapReleased = 0;
    input->tapSquare = -1;
}
