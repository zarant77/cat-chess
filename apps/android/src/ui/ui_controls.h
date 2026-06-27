#ifndef CAT_CHESS_UI_CONTROLS_H
#define CAT_CHESS_UI_CONTROLS_H

#include <stdint.h>

#include "../fonts/font_renderer.h"
#include "../renderer/renderer.h"

typedef struct {
    int x;
    int y;
    int width;
    int height;
} UiRect;

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
} UiNineSlice;

typedef enum {
    UI_BUTTON_PRIMARY = 0,
    UI_BUTTON_SECONDARY,
    UI_BUTTON_COMPACT,
    UI_BUTTON_DANGER,
    UI_BUTTON_DISABLED,
    UI_BUTTON_SELECTED
} UiButtonStyle;

#define UI_COLOR_BG_TOP 0xf5f0e7ffu
#define UI_COLOR_BG_BOTTOM 0xb8d2c7ffu
#define UI_COLOR_PANEL 0xf9f1e1eeu
#define UI_COLOR_PANEL_DARK 0x263244eeu
#define UI_COLOR_TEXT 0x27312bffu
#define UI_COLOR_TEXT_MUTED 0x5b6b63ffu
#define UI_COLOR_TEXT_ON_DARK 0xffffffffu
#define UI_COLOR_ACCENT 0xf4c542ffu
#define UI_COLOR_WARNING 0xb7791fffu
#define UI_COLOR_ERROR 0x8f3232ffu
#define UI_SPACE_XS 8
#define UI_SPACE_SM 12
#define UI_SPACE_MD 18
#define UI_SPACE_LG 28
#define UI_SPACE_XL 42
#define UI_BUTTON_HEIGHT_PRIMARY 86
#define UI_BUTTON_HEIGHT_SECONDARY 72
#define UI_BUTTON_HEIGHT_COMPACT 56
#define UI_BOARD_MARGIN 20

int ui_rect_contains(const UiRect* rect, int x, int y);
int ui_min_int(int a, int b);
int ui_max_int(int a, int b);
int ui_content_width(int screen_width);
int ui_top_safe_padding(int screen_height);
int ui_bottom_safe_padding(int screen_height);
int ui_centered_group_y(int screen_height, int content_height);
UiRect ui_centered_rect(int screen_width, int y, int width, int height);
void ui_draw_panel(Framebuffer* framebuffer, UiRect rect);
void ui_draw_card(Framebuffer* framebuffer, UiRect rect);
void ui_draw_nine_slice_panel(
        Framebuffer* framebuffer,
        const GeneratedSprite* sprite,
        UiRect rect,
        UiNineSlice slice
);
void ui_draw_button(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* label
);
void ui_draw_button_colored(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* label,
        uint32_t fill_color,
        uint32_t border_color
);
void ui_draw_button_scaled(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* label,
        int scale
);
void ui_draw_button_style(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* label,
        UiButtonStyle style,
        int scale
);
void ui_draw_label(
        Framebuffer* framebuffer,
        const PackedFont* font,
        int x,
        int y,
        int scale,
        uint32_t color,
        const char* label
);
void ui_draw_centered_label(
        Framebuffer* framebuffer,
        const PackedFont* font,
        int screen_width,
        int y,
        int scale,
        uint32_t color,
        const char* label
);
void ui_draw_screen_background(Framebuffer* framebuffer);
void ui_draw_screen_title(
        Framebuffer* framebuffer,
        const PackedFont* font,
        int screen_width,
        int y,
        const char* title
);
void ui_draw_status_pill(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* label,
        uint32_t color
);
void ui_draw_input_field(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* label,
        const char* value,
        int focused
);
void ui_draw_cat_mark(Framebuffer* framebuffer, int center_x, int y, int scale);
void ui_draw_slider(
        Framebuffer* framebuffer,
        const PackedFont* font,
        UiRect rect,
        const char* label,
        int value
);
int ui_slider_value_from_touch(UiRect rect, int touch_x);

#endif
