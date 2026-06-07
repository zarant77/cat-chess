#ifndef CAT_CHESS_RENDERER_H
#define CAT_CHESS_RENDERER_H

#include <android/native_window.h>
#include <stdint.h>

#include "../app/app_state.h"
#include "../sprites/generated_sprite.h"

typedef ANativeWindow_Buffer Framebuffer;

typedef enum {
    SPRITE_FIT_STRETCH = 0,
    SPRITE_FIT_CONTAIN = 1,
    SPRITE_FIT_COVER = 2
} SpriteFitMode;

void renderer_draw_color_rect(
        Framebuffer* framebuffer,
        int x,
        int y,
        int width,
        int height,
        uint32_t color
);

void renderer_fill_vertical_gradient(
        Framebuffer* framebuffer,
        uint32_t top_color,
        uint32_t bottom_color
);

void renderer_draw_generated_sprite_fit(
        Framebuffer* framebuffer,
        const GeneratedSprite* sprite,
        int dst_x,
        int dst_y,
        int dst_width,
        int dst_height,
        SpriteFitMode fit_mode
);

void renderer_draw_generated_sprite_tinted(
        Framebuffer* framebuffer,
        const GeneratedSprite* sprite,
        int dst_x,
        int dst_y,
        int dst_width,
        int dst_height,
        uint32_t tint_color
);

void renderer_draw_generated_sprite_palette(
        Framebuffer* framebuffer,
        const GeneratedSprite* sprite,
        int dst_x,
        int dst_y,
        int dst_width,
        int dst_height,
        uint32_t backing_color,
        uint32_t base_color
);
void renderer_draw_generated_sprite_region_scaled(
        Framebuffer* framebuffer,
        const GeneratedSprite* sprite,
        int src_x,
        int src_y,
        int src_width,
        int src_height,
        int dst_x,
        int dst_y,
        int dst_width,
        int dst_height
);

void renderer_draw_frame(ANativeWindow_Buffer* buffer, const AppState* app);

#endif
