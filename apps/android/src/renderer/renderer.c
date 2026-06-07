#include "renderer.h"

#include <stddef.h>
#include <stdint.h>

#include "../ui/screens/screen_create_game.h"
#include "../ui/screens/screen_home.h"
#include "../ui/screens/screen_join_game.h"
#include "../ui/screens/screen_local_game.h"
#include "../ui/screens/screen_my_games.h"
#include "../ui/screens/screen_settings.h"

static uint8_t rgba_r(uint32_t color) {
    return (uint8_t)((color >> 24) & 0xff);
}

static uint8_t rgba_g(uint32_t color) {
    return (uint8_t)((color >> 16) & 0xff);
}

static uint8_t rgba_b(uint32_t color) {
    return (uint8_t)((color >> 8) & 0xff);
}

static uint8_t rgba_a(uint32_t color) {
    return (uint8_t)(color & 0xff);
}

static uint32_t rgba_pack(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return ((uint32_t)r << 24)
            | ((uint32_t)g << 16)
            | ((uint32_t)b << 8)
            | (uint32_t)a;
}

static uint16_t rgba_to_rgb565(uint32_t color) {
    uint16_t r5 = (uint16_t)(rgba_r(color) >> 3);
    uint16_t g6 = (uint16_t)(rgba_g(color) >> 2);
    uint16_t b5 = (uint16_t)(rgba_b(color) >> 3);

    return (uint16_t)((r5 << 11) | (g6 << 5) | b5);
}

static uint32_t rgb565_to_rgba(uint16_t color) {
    uint8_t r = (uint8_t)((((color >> 11) & 0x1f) * 255) / 31);
    uint8_t g = (uint8_t)((((color >> 5) & 0x3f) * 255) / 63);
    uint8_t b = (uint8_t)(((color & 0x1f) * 255) / 31);

    return rgba_pack(r, g, b, 0xff);
}

static uint32_t renderer_blend_rgba(uint32_t src_color, uint32_t dst_color) {
    int src_a = (int)rgba_a(src_color);
    int inv_a = 255 - src_a;

    if (src_a <= 0) {
        return dst_color;
    }
    if (src_a >= 255) {
        return (src_color & 0xffffff00u) | 0xffu;
    }

    return rgba_pack(
            (uint8_t)(((int)rgba_r(src_color) * src_a + (int)rgba_r(dst_color) * inv_a) / 255),
            (uint8_t)(((int)rgba_g(src_color) * src_a + (int)rgba_g(dst_color) * inv_a) / 255),
            (uint8_t)(((int)rgba_b(src_color) * src_a + (int)rgba_b(dst_color) * inv_a) / 255),
            0xff
    );
}

static uint32_t framebuffer_read_rgba8888(const uint8_t* pixel) {
    return rgba_pack(pixel[0], pixel[1], pixel[2], pixel[3]);
}

static void framebuffer_write_rgba8888(uint8_t* pixel, uint32_t color) {
    pixel[0] = rgba_r(color);
    pixel[1] = rgba_g(color);
    pixel[2] = rgba_b(color);
    pixel[3] = 255;
}

static void renderer_write_pixel(
        Framebuffer* framebuffer,
        int x,
        int y,
        uint32_t color
) {
    if (framebuffer == 0
            || framebuffer->bits == 0
            || x < 0
            || y < 0
            || x >= framebuffer->width
            || y >= framebuffer->height
            || rgba_a(color) == 0) {
        return;
    }

    if (framebuffer->format == WINDOW_FORMAT_RGB_565) {
        uint16_t* pixels = (uint16_t*)framebuffer->bits;
        uint16_t* target = pixels + y * framebuffer->stride + x;

        if (rgba_a(color) < 255) {
            color = renderer_blend_rgba(color, rgb565_to_rgba(*target));
        }
        *target = rgba_to_rgb565(color);
        return;
    }

    {
        uint8_t* pixels = (uint8_t*)framebuffer->bits;
        uint8_t* target = pixels
                + (size_t)y * (size_t)framebuffer->stride * 4
                + (size_t)x * 4;

        if (rgba_a(color) < 255) {
            color = renderer_blend_rgba(color, framebuffer_read_rgba8888(target));
        }
        framebuffer_write_rgba8888(target, color);
    }
}

void renderer_draw_color_rect(
        Framebuffer* framebuffer,
        int x,
        int y,
        int width,
        int height,
        uint32_t color
) {
    int min_x = x;
    int min_y = y;
    int max_x = x + width;
    int max_y = y + height;

    if (framebuffer == 0 || framebuffer->bits == 0 || width <= 0 || height <= 0) {
        return;
    }

    if (min_x < 0) {
        min_x = 0;
    }
    if (min_y < 0) {
        min_y = 0;
    }
    if (max_x > framebuffer->width) {
        max_x = framebuffer->width;
    }
    if (max_y > framebuffer->height) {
        max_y = framebuffer->height;
    }

    for (int draw_y = min_y; draw_y < max_y; ++draw_y) {
        for (int draw_x = min_x; draw_x < max_x; ++draw_x) {
            renderer_write_pixel(framebuffer, draw_x, draw_y, color);
        }
    }
}

void renderer_fill_vertical_gradient(
        Framebuffer* framebuffer,
        uint32_t top_color,
        uint32_t bottom_color
) {
    int denominator;

    if (framebuffer == 0 || framebuffer->bits == 0) {
        return;
    }

    denominator = framebuffer->height > 1 ? framebuffer->height - 1 : 1;
    for (int y = 0; y < framebuffer->height; ++y) {
        uint32_t color = rgba_pack(
                (uint8_t)((int)rgba_r(top_color)
                        + (((int)rgba_r(bottom_color) - (int)rgba_r(top_color)) * y) / denominator),
                (uint8_t)((int)rgba_g(top_color)
                        + (((int)rgba_g(bottom_color) - (int)rgba_g(top_color)) * y) / denominator),
                (uint8_t)((int)rgba_b(top_color)
                        + (((int)rgba_b(bottom_color) - (int)rgba_b(top_color)) * y) / denominator),
                0xff
        );
        renderer_draw_color_rect(framebuffer, 0, y, framebuffer->width, 1, color);
    }
}

static void renderer_fit_rect(
        const GeneratedSprite* sprite,
        int* x,
        int* y,
        int* width,
        int* height,
        SpriteFitMode fit_mode
) {
    int dst_w = *width;
    int dst_h = *height;
    int fit_w;
    int fit_h;

    if (sprite == 0 || sprite->width <= 0 || sprite->height <= 0 || fit_mode == SPRITE_FIT_STRETCH) {
        return;
    }

    if (fit_mode == SPRITE_FIT_COVER) {
        if (dst_w * (int)sprite->height > dst_h * (int)sprite->width) {
            fit_w = dst_w;
            fit_h = (dst_w * (int)sprite->height) / (int)sprite->width;
        } else {
            fit_h = dst_h;
            fit_w = (dst_h * (int)sprite->width) / (int)sprite->height;
        }
    } else {
        if (dst_w * (int)sprite->height < dst_h * (int)sprite->width) {
            fit_w = dst_w;
            fit_h = (dst_w * (int)sprite->height) / (int)sprite->width;
        } else {
            fit_h = dst_h;
            fit_w = (dst_h * (int)sprite->width) / (int)sprite->height;
        }
    }

    *x += (dst_w - fit_w) / 2;
    *y += (dst_h - fit_h) / 2;
    *width = fit_w;
    *height = fit_h;
}

void renderer_draw_generated_sprite_tinted(
        Framebuffer* framebuffer,
        const GeneratedSprite* sprite,
        int dst_x,
        int dst_y,
        int dst_width,
        int dst_height,
        uint32_t tint_color
) {
    if (framebuffer == 0
            || sprite == 0
            || sprite->pixels == 0
            || dst_width <= 0
            || dst_height <= 0
            || sprite->width <= 0
            || sprite->height <= 0) {
        return;
    }

    for (int y = 0; y < dst_height; ++y) {
        int src_y = (y * (int)sprite->height) / dst_height;
        for (int x = 0; x < dst_width; ++x) {
            int src_x = (x * (int)sprite->width) / dst_width;
            uint32_t src = sprite->pixels[src_y * (int)sprite->width + src_x];
            uint8_t alpha = rgba_a(src);
            if (alpha > 0) {
                uint32_t color = (tint_color & 0xffffff00u) | alpha;
                renderer_write_pixel(framebuffer, dst_x + x, dst_y + y, color);
            }
        }
    }
}

void renderer_draw_generated_sprite_palette(
        Framebuffer* framebuffer,
        const GeneratedSprite* sprite,
        int dst_x,
        int dst_y,
        int dst_width,
        int dst_height,
        uint32_t backing_color,
        uint32_t base_color
) {
    if (framebuffer == 0
            || sprite == 0
            || sprite->pixels == 0
            || dst_width <= 0
            || dst_height <= 0
            || sprite->width <= 0
            || sprite->height <= 0) {
        return;
    }

    for (int y = 0; y < dst_height; ++y) {
        int src_y = (y * (int)sprite->height) / dst_height;
        for (int x = 0; x < dst_width; ++x) {
            int src_x = (x * (int)sprite->width) / dst_width;
            uint32_t src = sprite->pixels[src_y * (int)sprite->width + src_x];
            uint8_t alpha = rgba_a(src);

            if (alpha > 0) {
                int brightness = (int)rgba_r(src) + (int)rgba_g(src) + (int)rgba_b(src);
                uint32_t mapped = brightness >= 384 ? backing_color : base_color;
                uint32_t color = (mapped & 0xffffff00u) | alpha;
                renderer_write_pixel(framebuffer, dst_x + x, dst_y + y, color);
            }
        }
    }
}

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
) {
    if (framebuffer == 0
            || sprite == 0
            || sprite->pixels == 0
            || src_width <= 0
            || src_height <= 0
            || dst_width <= 0
            || dst_height <= 0) {
        return;
    }

    for (int y = 0; y < dst_height; ++y) {
        int sample_y = src_y + (y * src_height) / dst_height;
        if (sample_y < 0 || sample_y >= sprite->height) {
            continue;
        }

        for (int x = 0; x < dst_width; ++x) {
            int sample_x = src_x + (x * src_width) / dst_width;
            uint32_t src;

            if (sample_x < 0 || sample_x >= sprite->width) {
                continue;
            }

            src = sprite->pixels[sample_y * (int)sprite->width + sample_x];
            renderer_write_pixel(framebuffer, dst_x + x, dst_y + y, src);
        }
    }
}

void renderer_draw_generated_sprite_fit(
        Framebuffer* framebuffer,
        const GeneratedSprite* sprite,
        int dst_x,
        int dst_y,
        int dst_width,
        int dst_height,
        SpriteFitMode fit_mode
) {
    renderer_fit_rect(sprite, &dst_x, &dst_y, &dst_width, &dst_height, fit_mode);
    renderer_draw_generated_sprite_tinted(
            framebuffer,
            sprite,
            dst_x,
            dst_y,
            dst_width,
            dst_height,
            0xffffffffu
    );
}

void renderer_draw_frame(ANativeWindow_Buffer* buffer, const AppState* app) {
    if (buffer == 0 || buffer->bits == 0 || app == 0) {
        return;
    }

    renderer_fill_vertical_gradient(buffer, 0xf5f0e7ffu, 0xb8d2c7ffu);
    if (app->currentScreen == APP_SCREEN_HOME) {
        screen_home_render(buffer, app);
    } else if (app->currentScreen == APP_SCREEN_LOCAL_GAME) {
        screen_local_game_render(buffer, app);
    } else if (app->currentScreen == APP_SCREEN_CREATE_GAME) {
        screen_create_game_render(buffer, app);
    } else if (app->currentScreen == APP_SCREEN_JOIN_GAME) {
        screen_join_game_render(buffer, app);
    } else if (app->currentScreen == APP_SCREEN_MY_GAMES) {
        screen_my_games_render(buffer, app);
    } else if (app->currentScreen == APP_SCREEN_SETTINGS) {
        screen_settings_render(buffer, app);
    }
}
