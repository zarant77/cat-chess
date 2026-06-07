#ifndef CAT_CHESS_INPUT_H
#define CAT_CHESS_INPUT_H

typedef struct {
    int activePointerId;
    int touchActive;
    int tapReleased;
    int backPressed;
    float touchX;
    float touchY;
    float tapX;
    float tapY;
} InputState;

#define INPUT_TOUCH_DOWN 1
#define INPUT_TOUCH_MOVE 2
#define INPUT_TOUCH_UP 3
#define INPUT_TOUCH_CANCEL 4

void input_init(InputState* input);
void input_handle_touch(
        InputState* input,
        int action_type,
        int pointer_id,
        float x,
        float y,
        float screen_width
);
void input_end_frame(InputState* input);
void input_handle_back(InputState* input);

#endif
